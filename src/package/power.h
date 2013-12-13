#ifndef __POWER_H__
#define __POWER_H__

#include "package.h"
#include "card.h"

class PowerPackage: public Package{
    Q_OBJECT
        
public:
    PowerPackage();
};

class YimingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YimingCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DuanxieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuanxieCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class hegzhangjiaoskill3Card: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE hegzhangjiaoskill3Card();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif