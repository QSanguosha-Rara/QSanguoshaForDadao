#include "joypackage.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

class shitmove: public TriggerSkill{
public:
    shitmove(): TriggerSkill("shitmove"){
        global = true;
        events << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive() && target->getPhase() != Player::NotActive;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == player && (move.to_place == Player::DiscardPile || move.to_place == Player::PlaceTable || move.to_place == Player::PlaceSpecial)){
            int n = move.card_ids.length();
            QList<const Card *> shits;
            for (int i = 0; i < n; i ++){
                if (Sanguosha->getCard(move.card_ids[i])->isKindOf("Shit") && move.from_places[i] == Player::PlaceHand){
                    shits << Sanguosha->getCard(move.card_ids[i]);
                }
            }

            LogMessage l;
            l.from = player;

            foreach (const Card *shit, shits){
                l.card_str = QString::number(shit->getEffectiveId());
                
                if (shit->getSuit() == Card::Spade){
                    l.type = "$ShitLostHp";
                    room->sendLog(l);
                    room->loseHp(player);
                    continue;
                }

                DamageStruct shitdamage(shit, player, player);
                switch (shit->getSuit()){
                    case Card::Heart:
                        shitdamage.nature = DamageStruct::Fire;
                        break;
                    case Card::Diamond:
                        shitdamage.nature = DamageStruct::Normal;
                        break;
                    case Card::Club:
                        shitdamage.nature = DamageStruct::Thunder;
                        break;
                }

                l.type = "$ShitDamage";
                room->sendLog(l);

                room->damage(shitdamage);
            }
        }
        return false;
    }
};

/*

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = (ServerPlayer*)move.from;
    if(from && move.from_place == Player::PlaceHand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardPile
           || move.to_place == Player::PlaceSpecial
           || move.to_place == Player::PlaceTable)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.card_str = getEffectIdString();
        log.from = from;

        Room *room = from->getRoom();

        if(getSuit() == Spade){
            log.type = "$ShitLostHp";
            room->sendLog(log);

            room->loseHp(from);

            return;
        }

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        log.type = "$ShitDamage";
        room->sendLog(log);

        room->damage(damage);
    }
}
*/

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

// -----------  Deluge -----------------

Deluge::Deluge(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("deluge");

    judge.pattern = ".|.|1,13";
    judge.good = false;
    judge.reason = objectName();
}

void Deluge::takeEffect(ServerPlayer *target) const{
    QList<const Card *> cards = target->getCards("he");

    Room *room = target->getRoom();
    int n = qMin(cards.length(), target->aliveCount());
    if(n == 0)
        return;

    qShuffle(cards);
    cards = cards.mid(0, n);

    QList<int> card_ids;
    foreach(const Card *card, cards){
        card_ids << card->getEffectiveId();
        room->throwCard(card, NULL);
    }

    room->fillAG(card_ids);

    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    players << target;
    players = players.mid(0, n);
    foreach(ServerPlayer *player, players){
        if(player->isAlive()){
            int card_id = room->askForAG(player, card_ids, false, "deluge");
            card_ids.removeOne(card_id);

            room->takeAG(player, card_id);
        }
    }

    foreach(int card_id, card_ids)
        room->takeAG(NULL, card_id);

    room->clearAG();
}

// -----------  Typhoon -----------------

Typhoon::Typhoon(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("typhoon");

    judge.pattern = ".|diamond|2~9";
    judge.good = false;
    judge.reason = objectName();
}

void Typhoon::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    Sanguosha->playSystemAudioEffect("typhoon");
    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) == 1){
            int discard_num = qMin(6, player->getHandcardNum());
            if (discard_num != 0){
                room->askForDiscard(player, objectName(), discard_num, discard_num);
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Earthquake -----------------

Earthquake::Earthquake(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("earthquake");

    judge.pattern = ".|club|2~9";
    judge.good = false;
    judge.reason = objectName();
}

void Earthquake::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    Sanguosha->playSystemAudioEffect("earthquake");
    QList<ServerPlayer *> players = room->getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) <= 1){
            if (!player->getEquips().isEmpty()){
                player->throwAllEquips();
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Volcano -----------------

Volcano::Volcano(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("volcano");

    judge.pattern = ".|heart|2~9";
    judge.good = false;
    judge.reason = objectName();
}

void Volcano::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    Sanguosha->playSystemAudioEffect("volcano");
    QList<ServerPlayer *> players = room->getAllPlayers();

    foreach(ServerPlayer *player, players){
        int point = 3 - target->distanceTo(player);
        if(point >= 1){
            DamageStruct damage;
            damage.card = this;
            damage.damage = point;
            damage.to = player;
            damage.nature = DamageStruct::Fire;           
            room->damage(damage);
        }
    }
}

// -----------  MudSlide -----------------
MudSlide::MudSlide(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("mudslide");

    judge.pattern = ".|black|1,13,4,7";
    judge.good = false;
    judge.reason = objectName();
}

void MudSlide::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    Sanguosha->playSystemAudioEffect("mudslide");
    QList<ServerPlayer *> players = room->getAllPlayers();
    int to_destroy = 4;
    foreach(ServerPlayer *player, players){
        

        QList<const Card *> equips = player->getEquips();
        if(equips.isEmpty()){
            DamageStruct damage;
            damage.card = this;
            damage.to = player;
            room->damage(damage);
        }else{
            int n = qMin(equips.length(), to_destroy);
            for(int i = 0; i < n; i++){
                CardMoveReason reason(CardMoveReason::S_REASON_DISCARD, QString(), QString(), "mudslide");
                room->throwCard(equips.at(i), reason, player);
            }

            to_destroy -= n;
            if(to_destroy == 0)
                break;
        }
    }
}

class GrabPeach: public TriggerSkill{
public:
    GrabPeach():TriggerSkill("grab_peach"){
        events << CardUsed;
        global = true;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isKindOf("Peach")){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getOffensiveHorse() != NULL && p->getOffensiveHorse()->isKindOf("Monkey") && p->getMark("Equips_Nullified_to_Yourself") == 0 &&
                   p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getOffensiveHorse(), player, player);
                    p->broadcastSkillInvoke(objectName());
                    p->obtainCard(use.card);

                    use.to.clear();
                    data = QVariant::fromValue(use);
                }
            }
        }

        return false;
    }
};

Monkey::Monkey(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("Monkey");
}


QString Monkey::getCommonEffectName() const{
    return "Monkey";
}

class GaleShellSkill: public ArmorSkill{
public:
    GaleShellSkill():ArmorSkill("GaleShell"){
        events << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GaleShellDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++ damage.damage);
            player->getRoom()->sendLog(log);

            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

GaleShell::GaleShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("GaleShell");

    target_fixed = false;
}

bool GaleShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

DisasterPackage::DisasterPackage()
    :Package("Disaster")
{
    QList<Card *> cards;

    cards << new Deluge(Card::Spade, 1)
            << new Typhoon(Card::Spade, 4)
            << new Earthquake(Card::Club, 10)
            << new Volcano(Card::Heart, 13)
            << new MudSlide(Card::Heart, 7);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

JoyPackage::JoyPackage()
    :Package("Joy")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)
            << new Shit(Card::Spade, 10);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
    skills << new shitmove;
}

class YxSwordSkill: public WeaponSkill{
public:
    YxSwordSkill():WeaponSkill("YxSword"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->isKindOf("Slash")){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            QMutableListIterator<ServerPlayer *> itor(players);

            while(itor.hasNext()){
                itor.next();
                if(!player->inMyAttackRange(itor.value()))
                    itor.remove();
            }

            if(players.isEmpty())
                return false;

            QVariant victim = QVariant::fromValue(damage.to);
            room->setTag("YxSwordVictim", victim);
            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName(), "@yxsword-select", true, true);
            room->removeTag("YxSwordVictim");
            if (target != NULL){
                damage.from = target;
                data = QVariant::fromValue(damage);
                room->moveCardTo(player->getWeapon(), player, target, Player::PlaceHand,
                    CardMoveReason(CardMoveReason::S_REASON_TRANSFER, player->objectName(), objectName(), QString()));
            }
        }
        return damage.to->isDead();
    }
};

YxSword::YxSword(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("YxSword");
}

JoyEquipPackage::JoyEquipPackage()
    :Package("JoyEquip")
{
    (new Monkey(Card::Diamond, 5))->setParent(this);
    (new GaleShell(Card::Heart, 1))->setParent(this);
    (new YxSword)->setParent(this);

    type = CardPack;
    skills << new GaleShellSkill << new YxSwordSkill << new GrabPeach;
}

class Xianiao: public TriggerSkill{
public:
    Xianiao(): TriggerSkill("xianiao"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *xiahoujie = room->findPlayerBySkillName(objectName());
        if (xiahoujie == NULL || xiahoujie->isDead() || !player->inMyAttackRange(xiahoujie))
            return false;

        room->broadcastSkillInvoke(objectName());
        room->notifySkillInvoked(xiahoujie, objectName());

        xiahoujie->throwAllHandCards();
        xiahoujie->drawCards(player->getHp());

        return false;
    }
};

class Tangqiang: public TriggerSkill{
public:
    Tangqiang(): TriggerSkill("tangqiang"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (player == death.who && death.damage && death.damage->from){
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(player, objectName());

            room->loseMaxHp(death.damage->from, 1);
            room->acquireSkill(death.damage->from, objectName());
        }
        return false;
    }
};

DCPackage::DCPackage(): Package("DC"){
    General *xiahoujie = new General(this, "xiahoujie", "wei", 3);
    xiahoujie->addSkill(new Xianiao);
    xiahoujie->addSkill(new Tangqiang);

}

ADD_PACKAGE(Joy) 
ADD_PACKAGE(Disaster)
ADD_PACKAGE(JoyEquip)
ADD_PACKAGE(DC)