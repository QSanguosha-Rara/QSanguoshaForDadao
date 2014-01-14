#include "3d-package.h"
#include "skill.h"
#include "clientplayer.h"
#include "engine.h"
#include "wind.h"

class SanD1Chishen: public TriggerSkill{
public:
    SanD1Chishen(): TriggerSkill("sand1chishen"){
        events << AfterDrawNCards;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (!player->isKongcheng() && player->askForSkillInvoke(objectName())){
            player->throwAllHandCards();
        }
        return false;
    }
};

SanD1XinveCard::SanD1XinveCard(){
    mute = true;
    will_throw = false;
}

bool SanD1XinveCard::targetFixed() const{
    const Card *c = Self->tag["sand1xinve"].value<const Card *>();
    Card *_touse = Sanguosha->cloneCard(c->objectName(), Card::NoSuit, 0);
    if (_touse == NULL)
        return false;
    _touse->deleteLater();
    return _touse->targetFixed();
}

bool SanD1XinveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    const Card *c = Self->tag["sand1xinve"].value<const Card *>();
    Card *_touse = Sanguosha->cloneCard(c->objectName(), Card::NoSuit, 0);
    if (_touse == NULL)
        return false;
    _touse->deleteLater();
    return _touse->targetFilter(targets, to_select, Self);
}

bool SanD1XinveCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    const Card *c = Self->tag["sand1xinve"].value<const Card *>();
    Card *_touse = Sanguosha->cloneCard(c->objectName(), Card::NoSuit, 0);
    if (_touse == NULL)
        return false;
    _touse->deleteLater();
    return _touse->targetsFeasible(targets, Self);
}

const Card *SanD1XinveCard::validate(CardUseStruct &cardUse) const{
    Card *_touse = Sanguosha->cloneCard(user_string, Card::NoSuit, 0);
    _touse->setSkillName("sand1xinve");
    bool available = true;
    foreach (ServerPlayer *to, cardUse.to){
        if (cardUse.from->isProhibited(to, _touse)){
            available = false;
            break;
        }
    }
    available = available && _touse->isAvailable(cardUse.from);
    _touse->deleteLater();
    if (!available) return NULL;
    return _touse;
}

class SanD1Xinve: public ZeroCardViewAsSkill{
public:
    SanD1Xinve(): ZeroCardViewAsSkill("sand1xinve"){

    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("sand1xinve");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isKongcheng() && !player->hasUsed("SanD1XinveCard");
    }

    virtual const Card *viewAs() const{
        const Card *c = Self->tag["sand1xinve"].value<const Card *>();
        if (c != NULL){
            SanD1XinveCard *xinve = new SanD1XinveCard;
            xinve->setUserString(c->objectName());
            return xinve;
        }
        
        return NULL;
    }
};

class SanD1Mingzhan: public TriggerSkill{
public:
    SanD1Mingzhan(): TriggerSkill("sand1mingzhan"){
        events << TargetConfirmed << DamageDone << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL && player->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from == player && use.card != NULL && use.card->isKindOf("Slash") && player->askForSkillInvoke(objectName(), data)){
                QString choice = room->askForChoice(player, objectName(), "damage+nodamage", data);
                player->setFlags("sand1mingzhan_invoke" + use.card->toString());
                player->tag["sand1mingzhan_predict" + use.card->toString()] = (choice == "damage");
                player->tag["sand1mingzhan_real" + use.card->toString()] = false;
            }
        }
        else if (triggerEvent == DamageDone){
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card != NULL && damage.card->isKindOf("Slash") && damage.from->hasFlag("sand1mingzhan_invoke" + damage.card->toString())){
                damage.from->tag["sand1mingzhan_real" + damage.card->toString()] = true;
            }
        }
        else if (triggerEvent == CardFinished && TriggerSkill::triggerable(player)){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from == player && use.card != NULL && use.card->isKindOf("Slash") && player->hasFlag("sand1mingzhan_invoke" + use.card->toString())){
                player->setFlags("-sand1mingzhan_invoke" + use.card->toString());
                if (player->tag["sand1mingzhan_predict" + use.card->toString()].toBool() == player->tag["sand1mingzhan_real" + use.card->toString()].toBool()){
                    if (player->askForSkillInvoke(objectName(), "drawcard"))
                        player->drawCards(1);
                }

                player->tag.remove("sand1mingzhan_predict" + use.card->toString());
                player->tag.remove("sand1mingzhan_real" + use.card->toString());
            }
        }

        return false;
    }
};

class SanD1Xielu: public TriggerSkill{
public:
    SanD1Xielu(): TriggerSkill("sand1xielu"){
        events << TargetConfirmed << SlashEffected << BeforeCardsMove;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Slash")){
                foreach (ServerPlayer *to, use.to){
                    if (player->inMyAttackRange(to) && player->askForSkillInvoke(objectName(), QVariant::fromValue(to))){
                        bool can_discard_horse = false;
                        foreach (const Card *c, player->getEquips()){
                            if (c->isKindOf("Horse") && player->canDiscard(player, c->getEffectiveId())){
                                can_discard_horse = true;
                                break;
                            }
                        }
                        QString choice = "losehp";
                        if (can_discard_horse)
                            choice = room->askForChoice(player, objectName(), "losehp+discardhorse", QVariant::fromValue(to));

                        if (choice == "discardhorse")
                            room->askForCard(player, ".Horse!", "@sand1xielu-discard");
                        else {
                            room->loseHp(player, 1);
                            player->setFlags("sand1xielulosehp");
                            player->tag["sand1xielu_losehp"] = use.card->toString();
                            
                        }
                        room->setPlayerFlag(to, "sand1xieluinvoke");
                        room->setCardFlag(use.card, "sand1xieluinvoke");
                    }
                }
            }
        }
        else if (triggerEvent == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.slash != NULL && effect.slash->hasFlag("sand1xieluinvoke") && effect.to->hasFlag("sand1xieluinvoke")){
                room->setPlayerFlag(effect.to, "-sand1xieluinvoke");
                //danlaoavoid log
                return true;
            }
        }
        else if (triggerEvent == BeforeCardsMove && TriggerSkill::triggerable(player) && player->hasFlag("sand1xielulosehp")){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.reason.m_reason == CardMoveReason::S_REASON_USE && move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile){
                const Card *original_card = Card::Parse(player->tag["sand1xielu_losehp"].toString());
                bool real_slash = false;
                if (original_card->isVirtualCard()){
                    if (original_card->getSubcards() == move.card_ids)
                        real_slash = true;
                }
                else if (original_card->getId() == move.card_ids[0])
                    real_slash = true;

                if (real_slash){
                    player->setFlags("-sand1xielulosehp");
                    player->tag.remove("sand1xielu_losehp");

                    move.card_ids.clear();
                    move.from_places.clear();
                    data = QVariant::fromValue(move);

                    room->obtainCard(player, original_card);
                }
            }
        }
        return false;
    }
};

SanD1BianzhenCard::SanD1BianzhenCard(){

}

bool SanD1BianzhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

Player::Phase SanD1BianzhenCard::getPhaseFromString(const QString &s){
    if (s == "start")
        return Player::Start;
    else if (s == "judge")
        return Player::Judge;
    else if (s == "draw")
        return Player::Draw;
    else if (s == "discard")
        return Player::Discard;
    else if (s == "finish")
        return Player::Finish;
    else
        return Player::PhaseNone;

    return Player::PhaseNone;
}

void SanD1BianzhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (subcards.isEmpty())
        room->loseHp(effect.from);

    QString choice = room->askForChoice(effect.from, "sand1bianzhen", "start+judge+draw+discard+finish");

    Player::Phase phase = getPhaseFromString(choice);

    effect.from->setFlags("sand1bianzhen_used");
    effect.from->tag["sand1bianzhen_target"] = QVariant::fromValue(effect.to);
    effect.to->tag["sand1bianzhen_phase"] = static_cast<int>(phase);
}

class SanD1BianzhenVS: public ViewAsSkill{
public:
    SanD1BianzhenVS(): ViewAsSkill("sand1bianzhen"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("SanD1BianzhenCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty() || cards.length() == 2){
            SanD1BianzhenCard *c = new SanD1BianzhenCard;
            c->addSubcards(cards);
            return c;
        }
        return NULL;
    }
};

class SanD1Bianzhen: public TriggerSkill{
public:
    SanD1Bianzhen(): TriggerSkill("sand1bianzhen"){
        events << EventPhaseStart;
        view_as_skill = new SanD1BianzhenVS;
    }

    virtual int getPriority(TriggerEvent triggerEvent) const{
        return 1;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->hasFlag("sand1bianzhen_used") && player->getPhase() == Player::NotActive){
            ServerPlayer *target = player->tag["sand1bianzhen_target"].value<ServerPlayer *>();
            player->tag.remove("sand1bianzhen_target");
            if (target == NULL)
                return false;

            Player::Phase phase = static_cast<Player::Phase>(target->tag["sand1bianzhen_phase"].toInt());
            target->tag.remove("sand1bianzhen_phase");

            target->setPhase(phase);
            room->broadcastProperty(target, "phase");
            RoomThread *thread = room->getThread();
            try{
                if (!thread->trigger(EventPhaseStart, room, target))
                    thread->trigger(EventPhaseProceeding, room, target);
                thread->trigger(EventPhaseEnd, room, target);

                target->setPhase(Player::NotActive);
                room->broadcastProperty(target, "phase");
            }
            catch (TriggerEvent errorevent){
                if (errorevent == TurnBroken || errorevent == StageChange){
                    target->setPhase(Player::NotActive);
                    room->broadcastProperty(target, "phase");
                }
                throw errorevent;
            }
        }
        return false;
    }
};

class SanD1Congwen: public PhaseChangeSkill{
public:
    SanD1Congwen(): PhaseChangeSkill("sand1congwen"){
        frequency = Skill::Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getMark(objectName()) == 0
            && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        room->notifySkillInvoked(target, objectName());
        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$sand1congwen");

        QString choice = "draw";
        if (target->isWounded())
            choice = room->askForChoice(target, objectName(), "draw+recover");

        if (choice == "draw")
            target->drawCards(2);
        else {
            RecoverStruct recover;
            recover.who = target;
            room->recover(target, recover);
        }

        if (room->changeMaxHpForAwakenSkill(target)){
            room->addPlayerMark(target, objectName());
            room->acquireSkill(target, "wuyan");
        }
        return false;
    }
};

SanD1Package::SanD1Package(): Package("sand1"){

    General *miheng = new General(this, "sand1_miheng", "qun", 3);
    miheng->addSkill(new SanD1Chishen);
    miheng->addSkill(new SanD1Xinve);

    General *mushun = new General(this, "sand1_mushun", "qun", 4);
    mushun->addSkill(new SanD1Mingzhan);

    General *caoang = new General(this, "sand1_caoang", "wei", 4);
    caoang->addSkill(new SanD1Xielu);

    General *xushu = new General(this, "sand1_xushu", "shu", 4);
    xushu->addSkill(new SanD1Bianzhen);
    xushu->addSkill(new SanD1Congwen);


    addMetaObject<SanD1XinveCard>();
    addMetaObject<SanD1BianzhenCard>();
}

ADD_PACKAGE(SanD1)