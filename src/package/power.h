#ifndef __POWER_H__
#define __POWER_H__

#include "package.h"
#include "card.h"

class PowerPackage: public Package{
    Q_OBJECT
        
public:
    PowerPackage();
};

class hegzhangjiaoskill3Card: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE hegzhangjiaoskill3Card();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif