#include "hegemony.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "standard-skillcards.h"
#include "lingpackage.h"

class Shushen: public TriggerSkill {
public:
    Shushen(): TriggerSkill("shushen") {
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();
        int recover = recover_struct.recover;
        for (int i = 0; i < recover; i++) {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "shushen-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName(), target->getGeneralName().contains("liubei") ? 2 : 1);
                target->drawCards(1);
            } else {
                break;
            }
        }
        return false;
    }
};

class Shenzhi: public PhaseChangeSkill {
public:
    Shenzhi(): PhaseChangeSkill("shenzhi") {
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        Room *room = ganfuren->getRoom();
        if (ganfuren->getPhase() != Player::Start || ganfuren->isKongcheng())
            return false;
        if (room->askForSkillInvoke(ganfuren, objectName())) {
            // As the cost, if one of her handcards cannot be throwed, the skill is unable to invoke
            foreach (const Card *card, ganfuren->getHandcards()) {
                if (ganfuren->isJilei(card))
                    return false;
            }
            //==================================
            int handcard_num = ganfuren->getHandcardNum();
            room->broadcastSkillInvoke(objectName());
            ganfuren->throwAllHandCards();
            if (handcard_num >= ganfuren->getHp()) {
                RecoverStruct recover;
                recover.who = ganfuren;
                room->recover(ganfuren, recover);
            }
        }
        return false;
    }
};

class Duoshi: public OneCardViewAsSkill {
public:
    Duoshi(): OneCardViewAsSkill("duoshi") {
        filter_pattern = ".|red|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuoshiAE");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        AwaitExhausted *await = new AwaitExhausted(Card::NoSuit, 0);
        await->setSkillName(objectName());
        await->addSubcard(originalcard);
        return await;
    }
};

FenxunCard::FenxunCard() {
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->tag["FenxunTarget"] = QVariant::fromValue(effect.to);
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill: public OneCardViewAsSkill {
public:
    FenxunViewAsSkill(): OneCardViewAsSkill("fenxun") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("FenxunCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        FenxunCard *first = new FenxunCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Fenxun: public TriggerSkill {
public:
    Fenxun(): TriggerSkill("fenxun") {
        events << EventPhaseChanging << Death;
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->tag["FenxunTarget"].value<PlayerStar>() != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *dingfeng, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != dingfeng)
                return false;
        }
        ServerPlayer *target = dingfeng->tag["FenxunTarget"].value<PlayerStar>();

        if (target) {
            room->setFixedDistance(dingfeng, target, -1);
            dingfeng->tag.remove("FenxunTarget");
        }
        return false;
    }
};

class Mingshi: public TriggerSkill {
public:
    Mingshi(): TriggerSkill("mingshi") {
        events << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from) {
            if (damage.from->getCards("he").length() <= player->getCards("he").length() 
                    && room->askForDiscard(player, objectName(), 1, 1, true, true, "@mingshi-discard")) {
                room->notifySkillInvoked(player, objectName());
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#Mingshi";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(--damage.damage);
                room->sendLog(log);

                if (damage.damage < 1)
                    return true;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Lirang: public TriggerSkill {
public:
    Lirang(): TriggerSkill("lirang") {
        events << BeforeCardsMove;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *kongrong, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != kongrong)
            return false;
        if (move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {

            int i = 0;
            QList<int> lirang_card;
            foreach (int card_id, move.card_ids) {
                if (room->getCardOwner(card_id) == move.from
                    && (move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip)) {
                        lirang_card << card_id;
                }
                i++;
            }
            if (lirang_card.isEmpty())
                return false;

            QList<int> original_lirang = lirang_card;
            /*while (room->askForYiji(kongrong, lirang_card, objectName(), false, true, true, -1,
                                    QList<ServerPlayer *>(), move.reason, "@lirang-distribute", true)) {
                if (kongrong->isDead()) break;
            }*/
            room->askForRende(kongrong, lirang_card, objectName(), true, true, -1, 
                              QList<ServerPlayer *>(), move.reason, "@lirang-distribute", true);

            QList<int> ids = move.card_ids;
            i = 0;
            foreach (int card_id, ids) {
                if (original_lirang.contains(card_id) && !lirang_card.contains(card_id)) {
                    move.card_ids.removeOne(card_id);
                    move.from_places.removeAt(i);
                }
                i++;
            }
            data = QVariant::fromValue(move);
        }
        return false;
    }
};


Sijian::Sijian(): TriggerSkill("sijian") {
    events << CardsMoveOneTime;
}

bool Sijian::trigger(TriggerEvent, Room *room, ServerPlayer *tianfeng, QVariant &data) const{
    CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
    if (move.from == tianfeng && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard) {
        QList<ServerPlayer *> other_players = room->getOtherPlayers(tianfeng);
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, other_players) {
            if (tianfeng->canDiscard(p, "he"))
                targets << p;
        }
        if (targets.isEmpty()) return false;
        ServerPlayer *to = room->askForPlayerChosen(tianfeng, targets, objectName(), "sijian-invoke", true, true);
        if (to) {
            room->broadcastSkillInvoke(objectName(), to->isLord() ? 2 : 1);
            int card_id = room->askForCardChosen(tianfeng, to, getCardChosenFlag(), objectName(), false, Card::MethodDiscard);
            room->throwCard(card_id, to, tianfeng);
        }
    }
    return false;
}

QString Sijian::getCardChosenFlag() const{
    return "he";
}

class Suishi: public TriggerSkill {
public:
    Suishi(): TriggerSkill("suishi") {
        events << Dying << Death;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target = NULL;
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.damage && dying.damage->from)
                target = dying.damage->from;
            if (dying.who != player && target
                && room->askForSkillInvoke(target, objectName(), QString("draw:%1").arg(player->objectName()))) {
                room->broadcastSkillInvoke(objectName(), 1);
                if (target != player) {
                    room->notifySkillInvoked(player, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = target;
                    log.to << player;
                    log.arg = objectName();
                    room->sendLog(log);
                }

                player->drawCards(1);
            }
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.damage && death.damage->from)
                target = death.damage->from;
            if (target && room->askForSkillInvoke(target, objectName(), QString("losehp:%1").arg(player->objectName()))) {
                room->broadcastSkillInvoke(objectName(), 2);
                if (target != player) {
                    room->notifySkillInvoked(player, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = target;
                    log.to << player;
                    log.arg = objectName();
                    room->sendLog(log);
                }

                room->loseHp(player);
            }
        }
        return false;
    }
};

ShuangrenCard::ShuangrenCard() {
}

bool ShuangrenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ShuangrenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool success = effect.from->pindian(effect.to, "shuangren", NULL);
    if (success) {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *target, room->getAlivePlayers()) {
            if (effect.from->canSlash(target, NULL, false))
                targets << target;
        }
        if (targets.isEmpty())
            return;

        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "shuangren", "@dummy-slash");

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_shuangren");
        room->useCard(CardUseStruct(slash, effect.from, target), false);
    } else {
        room->broadcastSkillInvoke("shuangren", 3);
        room->setPlayerCardLimitation(effect.from, "use", "Slash", true);
    }
}

class ShuangrenViewAsSkill: public ZeroCardViewAsSkill {
public:
    ShuangrenViewAsSkill(): ZeroCardViewAsSkill("shuangren") {
        response_pattern = "@@shuangren";
    }

    virtual const Card *viewAs() const{
        return new ShuangrenCard;
    }
};

class Shuangren: public PhaseChangeSkill {
public:
    Shuangren(): PhaseChangeSkill("shuangren") {
        view_as_skill = new ShuangrenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *jiling) const{
        if (jiling->getPhase() == Player::Play && !jiling->isKongcheng()) {
            Room *room = jiling->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(jiling);
            foreach (ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke)
                room->askForUseCard(jiling, "@@shuangren", "@shuangren-card");
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (card->isKindOf("Slash"))
            return 2;
        else
            return 1;
    }
};

XiongyiCard::XiongyiCard() {
    mute = true;
}

bool XiongyiCard::targetFilter(const QList<const Player *> &, const Player *, const Player *) const{
    return true;
}

bool XiongyiCard::targetsFeasible(const QList<const Player *> &, const Player *) const{
    return true;
}

void XiongyiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (!use.to.contains(use.from))
        use.to << use.from;
    room->removePlayerMark(use.from, "@arise");
    room->broadcastSkillInvoke("xiongyi");
    room->doLightbox("$XiongyiAnimate", 4500);
    SkillCard::onUse(room, use);
}

void XiongyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *p, targets)
        p->drawCards(getDrawNum());
    if (targets.length() <= room->getAlivePlayers().length() / 2 && source->isWounded()) {
        RecoverStruct recover;
        recover.who = source;
        room->recover(source, recover);
    }
}

int XiongyiCard::getDrawNum() const{
    return 3;
}

class Xiongyi: public ZeroCardViewAsSkill {
public:
    Xiongyi(): ZeroCardViewAsSkill("xiongyi") {
        frequency = Limited;
        limit_mark = "@arise";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@arise") >= 1;
    }

    virtual const Card *viewAs() const{
        return new XiongyiCard;
    }
};

class Kuangfu: public TriggerSkill {
public:
    Kuangfu(): TriggerSkill("kuangfu") {
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *panfeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (damage.card && damage.card->isKindOf("Slash") && target->isAlive() && !target->hasFlag("Global_DebutFlag") && target->hasEquip() && !damage.chain && !damage.transfer) {
            QStringList equiplist;
            for (int i = 0; i <= 3; i++) {
                if (!target->getEquip(i)) continue;
                if (panfeng->canDiscard(target, target->getEquip(i)->getEffectiveId()) || panfeng->getEquip(i) == NULL)
                    equiplist << QString::number(i);
            }
            if (equiplist.isEmpty() || !panfeng->askForSkillInvoke(objectName(), data))
                return false;
            int equip_index = room->askForChoice(panfeng, "kuangfu_equip", equiplist.join("+"), QVariant::fromValue((PlayerStar)target)).toInt();
            const Card *card = target->getEquip(equip_index);
            int card_id = card->getEffectiveId();

            QStringList choicelist;
            if (panfeng->canDiscard(target, card_id))
                choicelist << "throw";
            if (equip_index > -1 && panfeng->getEquip(equip_index) == NULL)
                choicelist << "move";

            QString choice = room->askForChoice(panfeng, "kuangfu", choicelist.join("+"));

            if (choice == "move") {
                room->broadcastSkillInvoke(objectName(), 1);
                room->moveCardTo(card, panfeng, Player::PlaceEquip);
            } else {
                room->broadcastSkillInvoke(objectName(), 2);
                room->throwCard(card, target, panfeng);
            }
        }

        return false;
    }
};

class Huoshui: public TriggerSkill {
public:
    Huoshui(): TriggerSkill("huoshui") {
        events << EventPhaseStart << Death
            << EventLoseSkill << EventAcquireSkill
            << HpChanged << MaxHpChanged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority(TriggerEvent) const{
        return 5;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (!TriggerSkill::triggerable(player)
                || (player->getPhase() != Player::RoundStart || player->getPhase() != Player::NotActive)) return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player || !player->hasSkill(objectName())) return false;
        } else if (triggerEvent == EventLoseSkill) {
            if (data.toString() != objectName() || player->getPhase() == Player::NotActive) return false;
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() != objectName() || !player->hasSkill(objectName()) || player->getPhase() == Player::NotActive)
                return false;
        } else if (triggerEvent == MaxHpChanged || triggerEvent == HpChanged) {
            if (!room->getCurrent() || !room->getCurrent()->hasSkill(objectName())) return false;
        }

        if (player->getPhase() == Player::RoundStart || triggerEvent == EventAcquireSkill)
            room->broadcastSkillInvoke(objectName(), 1);
        else if (player->getPhase() == Player::NotActive || triggerEvent == EventLoseSkill)
            room->broadcastSkillInvoke(objectName(), 2);

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

        return false;
    }
};

class HuoshuiInvalidity: public InvaliditySkill {
public:
    HuoshuiInvalidity(): InvaliditySkill("#huoshui-inv") {
    }

    virtual bool isSkillValid(const Player *player, const Skill *skill) const{
        if (player->getPhase() == Player::NotActive) {
            const Player *current = NULL;
            foreach (const Player *p, player->getAliveSiblings()) {
                if (p->getPhase() != Player::NotActive) {
                    current = p;
                    break;
                }
            }
            if (current && current->hasSkill("huoshui")
                && player->getHp() >= (player->getMaxHp() + 1) / 2 && !skill->isAttachedLordSkill())
                return false;
        }
        return true;
    }
};

QingchengCard::QingchengCard() {
    handling_method = Card::MethodDiscard;
}

void QingchengCard::onUse(Room *room, const CardUseStruct &use) const{
    CardUseStruct card_use = use;
    ServerPlayer *player = card_use.from, *to = card_use.to.first();

    LogMessage log;
    log.from = player;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString();
    room->sendLog(log);

    QStringList skill_list;
    foreach (const Skill *skill, to->getVisibleSkillList()) {
        if (!skill_list.contains(skill->objectName()) && !skill->isAttachedLordSkill()) {
            skill_list << skill->objectName();
        }
    }
    QString skill_qc;
    if (!skill_list.isEmpty()) {
        QVariant data_for_ai = QVariant::fromValue((PlayerStar)to);
        skill_qc = room->askForChoice(player, "qingcheng", skill_list.join("+"), data_for_ai);
    }

    if (!skill_qc.isEmpty()) {
        LogMessage log;
        log.type = "$QingchengNullify";
        log.from = player;
        log.to << to;
        log.arg = skill_qc;
        room->sendLog(log);

        QStringList Qingchenglist = to->tag["Qingcheng"].toStringList();
        Qingchenglist << skill_qc;
        to->tag["Qingcheng"] = QVariant::fromValue(Qingchenglist);
        room->addPlayerMark(to, "Qingcheng" + skill_qc);

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);

        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, player, data);
    card_use = data.value<CardUseStruct>();

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), card_use.card->getSkillName(), QString());
    room->moveCardTo(this, player, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, player, data);
    thread->trigger(CardFinished, room, player, data);
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

class QingchengViewAsSkill: public OneCardViewAsSkill {
public:
    QingchengViewAsSkill(): OneCardViewAsSkill("qingcheng") {
        filter_pattern = "EquipCard!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        QingchengCard *first = new QingchengCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Qingcheng: public TriggerSkill {
public:
    Qingcheng(): TriggerSkill("qingcheng") {
        events << EventPhaseStart;
        view_as_skill = new QingchengViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority(TriggerEvent) const{
        return 6;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() == Player::RoundStart) {
            QStringList Qingchenglist = player->tag["Qingcheng"].toStringList();
            if (Qingchenglist.isEmpty()) return false;
            foreach (QString skill_name, Qingchenglist) {
                room->setPlayerMark(player, "Qingcheng" + skill_name, 0);
                if (player->hasSkill(skill_name)) {
                    LogMessage log;
                    log.type = "$QingchengReset";
                    log.from = player;
                    log.arg = skill_name;
                    room->sendLog(log);
                }
            }
            player->tag.remove("Qingcheng");
            foreach (ServerPlayer *p, room->getAllPlayers())
                room->filterCards(p, p->getCards("he"), true);

            Json::Value args;
            args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        }
        return false;
    }
};

class QingchengInvalidity: public InvaliditySkill {
public:
    QingchengInvalidity(): InvaliditySkill("#qingcheng-inv") {
    }

    virtual bool isSkillValid(const Player *player, const Skill *skill) const{
        return player->getMark("Qingcheng" + skill->objectName()) == 0;
    }
};

HegemonyPackage::HegemonyPackage()
    : Package("hegemony")
{

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false); // SHU 016
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    General *heg_luxun = new General(this, "heg_luxun", "wu", 3); // WU 007 G
    heg_luxun->addSkill("qianxun");
    heg_luxun->addSkill("lianying");
    heg_luxun->addSkill(new Duoshi);

    General *dingfeng = new General(this, "dingfeng", "wu"); // WU 016
    dingfeng->addSkill(new Skill("duanbing", Skill::Compulsory));
    dingfeng->addSkill(new Fenxun);

    General *mateng = new General(this, "mateng", "qun"); // QUN 013
    mateng->addSkill("mashu");
    mateng->addSkill(new Xiongyi);

    General *kongrong = new General(this, "kongrong", "qun", 3); // QUN 014
    kongrong->addSkill(new Mingshi);
    kongrong->addSkill(new Lirang);

    General *jiling = new General(this, "jiling", "qun", 4); // QUN 015
    jiling->addSkill(new Shuangren);
    jiling->addSkill(new SlashNoDistanceLimitSkill("shuangren"));
    related_skills.insertMulti("shuangren", "#shuangren-slash-ndl");

    General *tianfeng = new General(this, "tianfeng", "qun", 3); // QUN 016
    tianfeng->addSkill(new Sijian);
    tianfeng->addSkill(new Suishi);

    General *panfeng = new General(this, "panfeng", "qun"); // QUN 017
    panfeng->addSkill(new Kuangfu);

    General *zoushi = new General(this, "zoushi", "qun", 3, false); // QUN 018
    zoushi->addSkill(new Huoshui);
    zoushi->addSkill(new HuoshuiInvalidity);
    zoushi->addSkill(new Qingcheng);
    zoushi->addSkill(new QingchengInvalidity);
    related_skills.insertMulti("huoshui", "#huoshui-inv");
    related_skills.insertMulti("qingcheng", "#qingcheng-inv");

    addMetaObject<FenxunCard>();
    addMetaObject<ShuangrenCard>();
    addMetaObject<XiongyiCard>();
    addMetaObject<QingchengCard>();
}

ADD_PACKAGE(Hegemony)

