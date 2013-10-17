#include "assassinspackage.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "engine.h"


/*
MixinCard::MixinCard(){
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
}

bool MixinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MixinCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = source->getRoom();
    room->broadcastSkillInvoke("mixin", 1);
    target->obtainCard(this, false);
    QList<ServerPlayer *> others;
    foreach(ServerPlayer *p, room->getOtherPlayers(target))
        if(target->canSlash(p, NULL, false))
            others << p;

    if(others.isEmpty())
        return;

    ServerPlayer *target2 = room->askForPlayerChosen(source, others, "mixin");
    LogMessage log;
    log.type = "#CollateralSlash";
    log.from = source;
    log.to << target2;
    room->sendLog(log);
    if(room->askForUseSlashTo(target, target2, "#mixin", false)) {
        room->broadcastSkillInvoke("mixin", 2);
    }
    else {
        room->broadcastSkillInvoke("mixin", 3);
        QList<int> card_ids = target->handCards();
        room->fillAG(card_ids, target2);
        int cdid = room->askForAG(target2, card_ids, false, objectName());
        room->obtainCard(target2, cdid, false);
        room->clearAG(target2);
    }
    return;
}

class Mixin:public OneCardViewAsSkill{
public:
    Mixin():OneCardViewAsSkill("mixin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MixinCard");
    }

    virtual bool viewFilter(const Card *card) const{
        return !card->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        MixinCard *card = new MixinCard;
        card->addSubcard(originalCard);
        return card;
    }
};
*/

class Mixin: public PhaseChangeSkill{
public:
    Mixin(): PhaseChangeSkill("mixin"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Draw || !target->askForSkillInvoke(objectName()))
            return false;

        Room *room = target->getRoom();
        QList<int> cards = room->getNCards(3, false);
        CardsMoveStruct move(cards, NULL, Player::PlaceTable,
                CardMoveReason(CardMoveReason::S_REASON_TURNOVER, target->objectName()));
        room->moveCardsAtomic(move, true);
        room->getThread()->delay();
        room->getThread()->delay();

        room->fillAG(cards, target);
        int card_to_give = room->askForAG(target, cards, false, objectName());
        room->clearAG(target);

        cards.removeOne(card_to_give);
        DummyCard dummyobtain(cards);
        room->obtainCard(target, &dummyobtain, true);

        ServerPlayer *player_to_give = room->askForPlayerChosen(target, room->getOtherPlayers(target), objectName() + "-give", "@mixin-give");

        room->broadcastSkillInvoke(objectName(), 1);

        room->obtainCard(player_to_give, card_to_give, true);

        QList<ServerPlayer *> can_slashes;
        foreach(ServerPlayer *p, room->getOtherPlayers(player_to_give)){
            if (player_to_give->canSlash(p, false))
                can_slashes << p;
        }

        if (can_slashes.isEmpty())
            return true;

        ServerPlayer *player_to_slash = room->askForPlayerChosen(target, can_slashes, objectName() + "-slash", "@mixin-slash");

        LogMessage log;
        log.type = "#CollateralSlash";
        log.from = target;
        log.to << player_to_slash;
        room->sendLog(log);
        
        if (room->askForUseSlashTo(player_to_give, player_to_slash, "#mixin", false)){
            room->broadcastSkillInvoke("mixin", 2);
        }
        else {
            room->broadcastSkillInvoke("mixin", 3);
            QList<int> card_ids = player_to_give->handCards();
            room->fillAG(card_ids, player_to_slash);
            int cdid = room->askForAG(player_to_slash, card_ids, false, objectName());
            room->obtainCard(player_to_slash, cdid, false);
            room->clearAG(player_to_slash);
        }
        return true;
    }
};

class Cangni: public TriggerSkill{
public:
    Cangni():TriggerSkill("cangni"){
        events << EventPhaseStart << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == EventPhaseStart && player->getPhase() == Player::Discard && player->askForSkillInvoke(objectName())) {
            QStringList choices;
            choices << "draw";
            if(player->isWounded())
                choices << "recover";

            QString choice;
            if(choices.size() == 1)
                choice = choices.first();
            else
                choice = room->askForChoice(player, objectName(), choices.join("+"));

            if(choice == "recover") {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
            else
                player->drawCards(2);

            room->broadcastSkillInvoke("cangni", 1);
            player->turnOver();
            return false;
        }
        else if(triggerEvent == CardsMoveOneTime && !player->faceUp()) {
            if(player->getPhase() != Player::NotActive)
                return false;

            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            ServerPlayer *target = room->getCurrent();
            if(target->isDead())
                return false;

            if(move.from == player && move.to != player) {
                bool invoke = false;
                for(int i = 0; i < move.card_ids.size(); i++)
                    if(move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip) {
                        invoke = true;
                        break;
                    }

                room->setPlayerFlag(player, "cangnilose");    //for AI

                if(invoke && !target->isNude() && player->askForSkillInvoke(objectName())) {
                    room->broadcastSkillInvoke("cangni", 3);
                    room->askForDiscard(target, objectName(), 1, 1, false, true);
                }

                room->setPlayerFlag(player, "-cangnilose");    //for AI

                return false;
            }

            if(move.to == player && move.from != player)
                if(move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip){
                    room->setPlayerFlag(player, "cangniget");    //for AI

                    if(!target->hasFlag("cangni_used") && player->askForSkillInvoke(objectName())) {
                        room->setPlayerFlag(target, "cangni_used");
                        room->broadcastSkillInvoke("cangni", 2);
                        target->drawCards(1);
                    }

                    room->setPlayerFlag(player, "-cangniget");    //for AI
                }
        }

        return false;
    }
};

DuyiCard::DuyiCard(){
    target_fixed = true;
    mute = true;
}

void DuyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    int id = room->drawCard();

    CardsMoveStruct move(id, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_TURNOVER, source->objectName(), objectName(), QString()));
    room->moveCardsAtomic(move, true);
    room->getThread()->delay();
    room->getThread()->delay();

    source->tag["DuyiCardId"] = id;
    ServerPlayer *target = room->askForPlayerChosen(source, room->getAlivePlayers(), "duyi");
    source->tag.remove("DuyiCardId");

    const Card *card = Sanguosha->getCard(id);
    room->obtainCard(target, card, CardMoveReason(CardMoveReason::S_REASON_GOTBACK, source->objectName()));
    if (card->isBlack()) {
        room->setPlayerCardLimitation(target, "use,response", ".|.|.|hand", false);
        room->setPlayerMark(target, "duyi_target", 1);
        LogMessage log;
        log.type = "#duyi_eff";
        log.from = source;
        log.to << target;
        log.arg = "duyi";
        room->sendLog(log);
        room->broadcastSkillInvoke("duyi", 1);
    }
    else
        room->broadcastSkillInvoke("duyi", 2);
}

class DuyiViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuyiViewAsSkill():ZeroCardViewAsSkill("duyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuyiCard");
    }

    virtual const Card *viewAs() const{
        return new DuyiCard;
    }
};

class Duyi:public TriggerSkill{
public:
    Duyi():TriggerSkill("duyi"){
        view_as_skill = new DuyiViewAsSkill;
        events << EventPhaseChanging << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const {
        return target != NULL && target->hasInnateSkill(objectName());
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Death)
        {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }
        else
        {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }

        foreach(ServerPlayer *p, room->getAlivePlayers())
            if(p->getMark("duyi_target") > 0)
            {
                room->removePlayerCardLimitation(p, "use,response", ".|.|.|hand$0");
                room->setPlayerMark(p, "duyi_target", 0);
                LogMessage log;
                log.type = "#duyi_clear";
                log.from = p;
                log.arg = objectName();
                room->sendLog(log);
            }

        return false;
    }
};

class Duanzhi: public TriggerSkill{
public:
    Duanzhi(): TriggerSkill("duanzhi") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getTypeId() == Card::TypeSkill || use.from == player || !use.to.contains(player))
            return false;

        if(player->askForSkillInvoke(objectName(), data)) {
            room->setPlayerFlag(player, "duanzhi_InTempMoving");
            ServerPlayer *target = use.from;
            DummyCard *dummy = new DummyCard;
            QList<int> card_ids;
            QList<Player::Place> original_places;
            for (int i = 0; i < 2; i++) {
                if (!player->canDiscard(target, "he"))
                    break;
                if (room->askForChoice(player, objectName(), "discard+cancel") == "cancel")
                    break;
                card_ids << room->askForCardChosen(player, target, "he", objectName());
                original_places << room->getCardPlace(card_ids[i]);
                dummy->addSubcard(card_ids[i]);
                target->addToPile("#duanzhi", card_ids[i], false);
            }

            if (dummy->subcardsLength() > 0)
                for (int i = 0; i < dummy->subcardsLength(); i++)
                    room->moveCardTo(Sanguosha->getCard(card_ids[i]), target, original_places[i], false);

            room->setPlayerFlag(player, "-duanzhi_InTempMoving");

            if (dummy->subcardsLength() > 0)
                room->throwCard(dummy, target, player);
            delete dummy;
            room->loseHp(player);
        }
        return false;
    }
};

FengyinCard::FengyinCard(){
    target_fixed = true;
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
}

void FengyinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->getCurrent();
    target->obtainCard(this);
    room->broadcastSkillInvoke("fengyin");
    room->setPlayerFlag(target, "fengyin_target");
}

class FengyinViewAsSkill:public OneCardViewAsSkill{
public:
    FengyinViewAsSkill():OneCardViewAsSkill("fengyin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fengyin";
    }

    virtual bool viewFilter(const Card *card) const{
        return card->isKindOf("Slash");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FengyinCard *card = new FengyinCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Fengyin:public TriggerSkill{
public:
    Fengyin():TriggerSkill("fengyin"){
        view_as_skill = new FengyinViewAsSkill;
        events << EventPhaseChanging << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer || splayer == player)
            return false;

        if(triggerEvent == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::Start)
            if(player->getHp() >= splayer->getHp())
                room->askForUseCard(splayer, "@@fengyin", "@fengyin", -1, Card::MethodNone);

        if(triggerEvent == EventPhaseStart && player->hasFlag("fengyin_target")){
            player->skip(Player::Play);
            player->skip(Player::Discard);
        }

        return false;
    }
};

class ChizhongKeep: public MaxCardsSkill{
public:
    ChizhongKeep():MaxCardsSkill("chizhong"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return target->getLostHp();
        else
            return 0;
    }
};

class Chizhong: public TriggerSkill{
public:
    Chizhong():TriggerSkill("#chizhong"){
        events << Death << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(triggerEvent == EventPhaseStart && splayer == player && player->getPhase() == Player::Discard) {
            if(player->getHandcardNum() > player->getHp() && player->isWounded()){
                LogMessage log;
                log.type = "#Chizhong";
                log.from = splayer;
                log.arg = "chizhong";
                room->sendLog(log);
                room->broadcastSkillInvoke("chizhong", 1);
            }
            return false;
        }

        if(triggerEvent == Death && TriggerSkill::triggerable(player))
        {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who == player)
                return false;

            room->setPlayerProperty(splayer, "maxhp", splayer->getMaxHp()+1);
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = splayer;
            log.arg = "chizhong";
            room->sendLog(log);
            room->broadcastSkillInvoke("chizhong", 2);
        }
        return false;
    }
};

AssassinsPackage::AssassinsPackage(): Package("assassins") {

    //todo: these three generals are to move to tigerfly package
    
    General *fuhuanghou = new General(this, "as_fuhuanghou", "qun", 3, false);
    fuhuanghou->addSkill(new Mixin);
    fuhuanghou->addSkill(new Cangni);

    General *jiben = new General(this, "as_jiben", "qun", 3);
    jiben->addSkill(new Duyi);
    jiben->addSkill(new Duanzhi);
    jiben->addSkill(new FakeMoveSkill("duanzhi"));
    related_skills.insertMulti("duanzhi", "#duanzhi-fake-move");

    General *fuwan = new General(this, "as_fuwan", "qun", 3);
    fuwan->addSkill(new Fengyin);
    fuwan->addSkill(new ChizhongKeep);
    fuwan->addSkill(new Chizhong);
    related_skills.insertMulti("chizhong", "#chizhong");

    //SPconvert from SP fuwan to as_mushun
    //consider moving as_mushun to test package

    General *mushun = new General(this, "as_mushun", "qun", 4, true, true);
    mushun->addSkill("moukui");

    //hide these two generals because the skills and the cards are same with the ones in SP package
    //and I don't want to move the codes to sp-package.h/.cpp

    //consider move these skills to sp-package.h/.cpp

    //to delete


    
    /*addMetaObject<MixinCard>();*/
    addMetaObject<DuyiCard>();
    addMetaObject<FengyinCard>();
}

ADD_PACKAGE(Assassins)
