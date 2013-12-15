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
        //frequency = Frequent;
    }

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

            room->askForGuanxing(target, getcards, Room::GuanxingDownOnly);

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

                ServerPlayer *current = room->getCurrent(); //��ǰ�غϽ�ɫ��ǹ
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


class Guixiu: public MaxCardsSkill{
public:
    Guixiu(): MaxCardsSkill("guixiu"){

    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill(objectName())){
            return 2;
        }
        return 0;
    }
};

class GuixiuInitial: public DrawCardsSkill{
public:
    GuixiuInitial(): DrawCardsSkill("#guixiu-initial", true){

    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        return n + 2;
    }
};

class Cunsi: public TriggerSkill{
public:
    Cunsi(): TriggerSkill("cunsi"){
        events << Dying;
        frequency = Limited;
        limit_mark = "@mifuren";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@mifuren") > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who == player && player->askForSkillInvoke(objectName())){
            player->loseAllMarks("@mifuren");
            RecoverStruct recover;
            recover.who = player;
            recover.recover = 2 - player->getHp();
            room->recover(player, recover);
            room->handleAcquireDetachSkills(player, "yiming");
        }
        return false;
    }
};

class Yongjue: public TriggerSkill{
public:
    Yongjue(): TriggerSkill("yongjue"){
        events << CardUsed << CardResponded;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (!player->hasFlag("yongjue")){
            const Card *card = NULL;
            if (triggerEvent == CardUsed){
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.card != NULL && !use.card->isKindOf("SkillCard"))
                    card = use.card;
            }
            else{
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse)
                    card = resp.m_card;
            }

            if (card != NULL && card->isKindOf("Slash")){
                ServerPlayer *mifuren = room->findPlayerBySkillName(objectName());
                if (mifuren != NULL && mifuren->askForSkillInvoke(objectName(), QVariant::fromValue(player)))
                    player->obtainCard(card);
            }
        }
        return false;
    }
};

YimingCard::YimingCard(){

}

void YimingCard::onEffect(const CardEffectStruct &effect) const{
    QList<Player::Phase> phaselist;
    switch (getSuit()){
        case Card::Club:{
            phaselist << Player::Discard;
            break;
        }
        case Card::Diamond:{
            phaselist << Player::Draw;
            break;
        }
        case Card::Heart:{
            phaselist << Player::Play;
            break;
        }
        case Card::Spade:{
            phaselist << Player::Start << Player::Finish;
            break;
        }
        default:
            Q_ASSERT(false);
    }
    effect.to->play(phaselist);
}

class YimingVS: public OneCardViewAsSkill{
public:
    YimingVS(): OneCardViewAsSkill("yiming"){
        response_pattern = "@@yiming";
        filter_pattern = ".!";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        YimingCard *c = new YimingCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class Yiming: public PhaseChangeSkill{
public:
    Yiming(): PhaseChangeSkill("yiming"){
        view_as_skill = new YimingVS;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::NotActive)
            target->getRoom()->askForUseCard(target, "@@yiming", "@yiming-prompt", -1, Card::MethodDiscard);
        return false;
    }
};

DuanxieCard::DuanxieCard(){

}

void DuanxieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setPlayerProperty(effect.to, "chained", true);
    room->setPlayerProperty(effect.from, "chained", true);
}

class Duanxie: public ZeroCardViewAsSkill{
public:
    Duanxie(): ZeroCardViewAsSkill("duanxue"){

    }

    virtual const Card *viewAs() const{
        return new DuanxieCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuanxueCard");
    }
};

class Fenming: public PhaseChangeSkill{
public:
    Fenming(): PhaseChangeSkill("fenming"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Finish && target->isChained() && target->askForSkillInvoke(objectName())){
            Room *room = target->getRoom();
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if (p->isChained() && target->canDiscard(p, "he")){
                    int to_discard = room->askForCardChosen(target, p, "he", objectName(), p == target, Card::MethodDiscard);
                    room->throwCard(to_discard, p, target);
                }
            }
        }
        return false;
    }
};

class hegzhangjiaoskill1: public PhaseChangeSkill{
public:
    hegzhangjiaoskill1(): PhaseChangeSkill("hegzhangjiaoskill1"){
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Draw){
            Room *room = target->getRoom();

            int qunplayers = 0;
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if (p->getKingdom() == "qun")
                    qunplayers ++;

            if (qunplayers <= 1)
                return false;

            if (target->askForSkillInvoke(objectName())){
                QList<int> guanxing_cards = room->getNCards(qunplayers);
                room->askForGuanxing(target, guanxing_cards, Room::GuanxingUpOnly);
            }

        }
        return false;
    }
};

class hegzhangjiaoskill2: public PhaseChangeSkill{
public:
    hegzhangjiaoskill2(): PhaseChangeSkill("hegzhangjiaoskill2$"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive() && target->hasLordSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Start && target->getPile("skysoldier").length() == 0){
            Room *room = target->getRoom();

            int qunplayers = 0;
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if (p->getKingdom() == "qun")
                    qunplayers ++;

            if (qunplayers == 0)
                return false;

            QList<int> skill2cards = room->getNCards(qunplayers);
            CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, target->objectName(), objectName(), QString());
            CardsMoveStruct move(skill2cards, NULL, Player::PlaceTable, reason);
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();
            room->getThread()->delay();

            target->addToPile("skysoldier", skill2cards, true);

        }
        return false;
    }
};

hegzhangjiaoskill3Card::hegzhangjiaoskill3Card(){
    target_fixed = true;
}

void hegzhangjiaoskill3Card::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    const Card *tpys = NULL;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        foreach(const Card *card, p->getEquips()){
            if (Sanguosha->getEngineCard(card->getEffectiveId())->isKindOf("taipingyaoshu")){
                tpys = Sanguosha->getCard(card->getEffectiveId());
                break;
            }
        }
        if (tpys != NULL)
            break;
        foreach(const Card *card, p->getJudgingArea()){
            if (Sanguosha->getEngineCard(card->getEffectiveId())->isKindOf("taipingyaoshu")){
                tpys = Sanguosha->getCard(card->getEffectiveId());
                break;
            }
        }
        if (tpys != NULL)
            break;
    }
    if (tpys == NULL)
        foreach(int id, room->getDiscardPile()){
            if (Sanguosha->getEngineCard(id)->isKindOf("taipingyaoshu")){
                tpys = Sanguosha->getCard(id);
                break;
            }
        }
        
    if (tpys == NULL)
        return;

    source->obtainCard(tpys, true);
}

class hegzhangjiaoskill3: public OneCardViewAsSkill{
public:
    hegzhangjiaoskill3(): OneCardViewAsSkill("hegzhangjiaoskill3"){
        filter_pattern = ".|red!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("hegzhangjiaoskill3Card");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        hegzhangjiaoskill3Card *c = new hegzhangjiaoskill3Card;
        c->addSubcard(originalCard);
        return c;
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

    General *mifuren = new General(this, "mifuren", "shu", 2);
    mifuren->addSkill(new Guixiu);
    mifuren->addSkill(new GuixiuInitial);
    mifuren->addSkill(new Yongjue);
    mifuren->addSkill(new Cunsi);
    skills << new Yiming;
    mifuren->addRelateSkill("yiming");
    related_skills.insertMulti("guixiu", "#guixiu-initial");

    General *chenwudongxi = new General(this, "chenwudongxi", "wu", 4);
    chenwudongxi->addSkill(new Duanxie);
    chenwudongxi->addSkill(new Fenming);

    General *zhangjiao = new General(this, "heg_zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new hegzhangjiaoskill1);
    zhangjiao->addSkill(new hegzhangjiaoskill2);
    zhangjiao->addSkill(new hegzhangjiaoskill3);

    addMetaObject<YimingCard>();
    addMetaObject<DuanxieCard>();
    addMetaObject<hegzhangjiaoskill3Card>();
}

ADD_PACKAGE(Power)