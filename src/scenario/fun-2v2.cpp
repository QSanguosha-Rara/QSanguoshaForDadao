#include "fun-2v2.h"
#include "engine.h"

class Fun2v2Rule: public ScenarioRule{
public:
    Fun2v2Rule(Scenario *scenario): ScenarioRule(scenario){
        events << GameStart << BuryVictim << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == GameStart && player == NULL){
/*
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *p, players)
                if (p->getRole() == "rebel" && p->getNext()->getRole() == "rebel")
                    room->setCurrent(p->getNext());
*/

            room->setTag("SkipNormalDeathProcess", true);

            return false;
        }

        if (player == NULL)
            return false;

        if (triggerEvent == GameOverJudge){
            //todo:gameoverjudge

            QStringList loyalists;
            QStringList rebels;

            foreach(ServerPlayer *p, room->getAlivePlayers())
                if (p->isAlive() && p != player)
                    if (p->getRole() == "loyalist")
                        loyalists << p->objectName();
                    else
                        rebels << p->objectName();

            if (loyalists.isEmpty())
                room->gameOver("rebel");
            else if (rebels.isEmpty())
                room->gameOver("loyalist");

            return true;
        }
        else {
            player->bury();
            QString role = player->getRole();
            
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if (p->getRole() == role)
                    p->drawCards(1);
        }
        return false;
    }
};

Fun2v2::Fun2v2(): Scenario("fun2v2"){
    rule = new Fun2v2Rule(this);
}

void Fun2v2::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "rebel" << "loyalist" << "loyalist" << "rebel";
}

int Fun2v2::getPlayerCount() const{
    return 4;
}

QString Fun2v2::getRoles() const{
    return "FCCF";
}

void Fun2v2::onTagSet(Room *room, const QString &key) const{
    return ;
}

bool Fun2v2::generalSelection() const{
    return true;
}

AI::Relation Fun2v2::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    return a->getRole() == b->getRole() ? AI::Friend : AI::Enemy;
}