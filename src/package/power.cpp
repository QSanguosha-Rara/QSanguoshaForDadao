#include "power.h"
#include "skill.h"
#include "ai.h"
#include "jsonutils.h"
#include "util.h"
#include "engine.h"
#include "settings.h"

class Xunxun: public PhaseChangeSkill{
public:
    Xunxun(): PhaseChangeSkill("xunxun"){
        //frequency = Compulsory;
    }

private:
    static void askForXunxun(ServerPlayer *lidian, const QList<int> &cards){
        Room *room = lidian->getRoom();
        QList<int> top_cards, bottom_cards;
        room->notifyMoveFocus(lidian, QSanProtocol::S_COMMAND_SKILL_GUANXING);

        AI *ai = lidian->getAI();
        if (ai) {
            ai->askForGuanxing(cards, top_cards, bottom_cards, true);
        } else if (cards.length() == 1) {
            top_cards = cards;
        } else {
            Json::Value guanxingArgs(Json::arrayValue);
            guanxingArgs[0] = QSanProtocol::Utils::toJsonArray(cards);
            guanxingArgs[1] = true;
            bool success = room->doRequest(lidian, QSanProtocol::S_COMMAND_SKILL_GUANXING, guanxingArgs, true);
            if (!success) {
                foreach (int card_id, cards)
                    room->getDrawPile().append(card_id);
                return;
            }
            Json::Value clientReply = lidian->getClientReply();
            if (clientReply.isArray() && clientReply.size() == 2) {
                success &= QSanProtocol::Utils::tryParse(clientReply[0], top_cards);
                success &= QSanProtocol::Utils::tryParse(clientReply[1], bottom_cards);
            }
        }

        bool length_equal = top_cards.length() == cards.length();
        bool result_equal = top_cards.toSet() == cards.toSet();
        if (!length_equal || !result_equal) {
            top_cards = cards;
            bottom_cards.clear();
        }
        if (!top_cards.isEmpty()) {
            LogMessage log;
            log.type = "$GuanxingBottom";
            log.from = lidian;
            log.card_str = IntList2StringList(top_cards).join("+");
            room->doNotify(lidian, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
        }

        QListIterator<int> i(top_cards);
        while (i.hasNext())
            room->getDrawPile().append(i.next());


        room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, Json::Value(room->getDrawPile().length()));
    }

public:
    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Draw && target->askForSkillInvoke(objectName())){
            Room *room = target->getRoom();
            QList<int> getcards = room->getNCards(4);
            QList<int> origin_cards = getcards;
            QList<int> gained;

            int aidelay = Config.AIDelay;
            Config.AIDelay = 0;
            for (int i = 1; i <= 2; i++){
                room->fillAG(origin_cards, target, gained);
                int gain = room->askForAG(target, getcards, false, objectName());
                room->clearAG(target);
                getcards.removeOne(gain);
                gained << gain;
            }
            Config.AIDelay = aidelay;

            DummyCard gaindummy(gained);
            room->obtainCard(target, &gaindummy, false);

            askForXunxun(target, getcards);  //temp method for this skill, waiting for Para's update

            return true;
        }
        return false;
    }
};

class Wangxi: public TriggerSkill{
public:
    Wangxi(): TriggerSkill("wangxi"){
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target;
        DamageStruct damage = data.value<DamageStruct>();
        if (triggerEvent == Damage)
            target = damage.to;
        else
            target = damage.from;
        if (target == NULL || target->isDead())
            return false;

        int x = damage.damage;
        for (int i = 0; i < x; i ++){
            if (player->isDead() || !room->askForSkillInvoke(player, objectName()))
                return false;

            QList<int> cards = room->getNCards(2, false, true);
            room->fillAG(cards, player);
            int id = room->askForAG(player, cards, false, objectName());
            room->obtainCard(player, id, false);
            room->clearAG(player);

            cards.removeOne(id);
            int to_give = cards.first();
            room->obtainCard(target, Sanguosha->getCard(to_give), CardMoveReason(CardMoveReason::S_REASON_PREVIEWGIVE, player->objectName()), false);

            if (target->isDead())
                return false;
        }
        return false;
    }
};

class Hengjiang: public TriggerSkill{ //temp version
public:
    Hengjiang(): TriggerSkill("hengjiang"){
        events << Damaged << CardsMoveOneTime << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        switch (triggerEvent){
            case (Damaged):{
                if (!TriggerSkill::triggerable(player))
                    return false;

                DamageStruct damage = data.value<DamageStruct>();

                ServerPlayer *current = room->getCurrent(); //当前回合角色躺枪
                if (current && current->isAlive() && current->getPhase() != Player::NotActive){
                    player->tag["hengjiang_damage"] = data;
                    if (player->askForSkillInvoke(objectName(), "maxcard:" + current->objectName()))
                        current->gainMark("@hengjiangmaxcard", damage.damage);
                    player->tag.remove("hengjiang_damage");
                }

                break;
            }
            case (CardsMoveOneTime):{
                if (!TriggerSkill::triggerable(player))
                    return false;

                CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
                if (move.from && move.from->getPhase() == Player::Discard && move.from_places.contains(Player::PlaceHand) &&
                        (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD){
                    ServerPlayer *from = (ServerPlayer *)move.from;
                    room->setPlayerMark(from, "hengjiang_discard", 1);
                }
                break;
            }
            case (EventPhaseStart):{
                if (player->getPhase() != Player::NotActive)
                    return false;
                player->loseAllMarks("@hengjiangmaxcard");
                ServerPlayer *lidian = room->findPlayerBySkillName(objectName());
                if (lidian == NULL || lidian->isDead())
                    return false;

                if (player->getMark("hengjiang_discard") == 0){
                    bool drawflag = false;
                    foreach(ServerPlayer *p, room->getOtherPlayers(lidian))
                        if (p->getHandcardNum() > lidian->getHandcardNum()){
                            drawflag = true;
                            break;
                        }
                    if (drawflag && player->askForSkillInvoke(objectName(), "drawcard"))
                        lidian->drawCards(1);
                }
                room->setPlayerMark(player, "hengjiang_discard", 0);
                
                break;
            }
        }
        return false;
    }
};
class HengjiangMaxCards: public MaxCardsSkill{
public:
    HengjiangMaxCards(): MaxCardsSkill("#hengjiang"){

    }

    virtual int getExtra(const Player *target) const{
        return -target->getMark("@hengjiangmaxcard");
    }
};

PowerPackage::PowerPackage(): Package("Power"){

    General *lidian = new General(this, "lidian", "wei", 3);
    lidian->addSkill(new Xunxun);
    lidian->addSkill(new Wangxi);

    General *zangba = new General(this, "zangba", "wei");
    zangba->addSkill(new Hengjiang);
    zangba->addSkill(new HengjiangMaxCards);
    related_skills.insertMulti("hengjiang", "#hengjiang");

}

ADD_PACKAGE(Power)