#ifndef __FUN2V2_H_
#define __FUN2V2_H_

#include "scenario.h"
#include "roomthread.h"

class ServerPlayer;

class Fun2v2: public Scenario{
    Q_OBJECT

public:
    explicit Fun2v2();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

};

#endif