#ifndef _SP_H
#define _SP_H

#include "package.h"
#include "card.h"
#include "standard.h"

class SPPackage: public Package {
    Q_OBJECT

public:
    SPPackage();
};

class ChaosPackage: public Package {
    Q_OBJECT

public:
    ChaosPackage();
};

class OLPackage: public Package {
    Q_OBJECT

public:
    OLPackage();
};

class Yongsi: public TriggerSkill {
    Q_OBJECT

public:
    Yongsi();
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *yuanshu, QVariant &data) const;

protected:
    virtual int getKingdoms(ServerPlayer *yuanshu) const;
};

class WeidiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE WeidiCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YuanhuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE YuanhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuejiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XuejiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MizhaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE MizhaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BifaCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE BifaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SongciCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE SongciCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AocaiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE AocaiCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class DuwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DuwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhoufuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhoufuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HongyuanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HongyuanCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JisuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JisuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#include "skill.h"
class Fentian: public PhaseChangeSkill {
public:
    Fentian();
    virtual bool onPhaseChange(ServerPlayer *hanba) const;
};

class Zhiri: public PhaseChangeSkill {
public:
    Zhiri();
    virtual bool onPhaseChange(ServerPlayer *hanba) const;
};

class XintanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XintanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SPCardPackage: public Package {
    Q_OBJECT

public:
    SPCardPackage();
};

class SPMoonSpear: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE SPMoonSpear(Card::Suit suit = Diamond, int number = 12);
};

#endif

