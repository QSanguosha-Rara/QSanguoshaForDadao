--技能讲解2：技能牌SkillCard

--神杀中，技能的效果在很多时候都技能牌实现。即，把技能定义在一张没有实体的抽象“牌”当中，当你发动技能的时候，视为你使用了这张牌。

--对于指定对象发动的技能，对象的指定也算在牌的效果当中。
--很多游戏的技能发动都带有cost这个概念，即发动技能的代价。神杀中，cost只能是你的牌或装备；也就是说，
--cost只能靠ViewAsSkill来实现。如果想实现类似于“发动代价”这样的效果，请用“发动技能的负面效果”这样的概念来替换。

--由于技能牌的需要有多个实例存在（每次发动技能得到一个技能牌），
--我们在DIY module当中并不像ViewAsSkill和TriggerSkill当中使用构造函数来创建SkillCard。
--我们需要将SkillCard的参数在一个lua table当中定义好，然后在每次需要创建SkillCard的时候再调用sgs.CreateSkillCard获取SkillCard对象。
--或者，我们也可以先创建好一个SkillCard，然后在技能中复制它。

--sgs.CreateSkillCard需要以下参数定义：

--name, target_fixed, will_throw, can_recast, handling_method, mute, filter, feasible, about_to_use, on_use, on_effect, on_validate, on_validate_in_response

--name:
--字符串，牌的名字。取个好听的名字~
--没有默认值。快去取名字……

--target_fixed：
--布尔值，使用该牌时是否需要玩家指定目标。
--默认为false，使用时你需要指定目标，然后点确定。

--will_throw:
--布尔值，该牌在使用后是否被弃置。还记得subCards吗？
--对于拼点技能，请将will_throw设为false，否则对方将看到你的牌之后再选择拼点牌。
--也可以将will_throw设为false,然后使用room:throwCard(card)这个方法来灵活地控制如何将牌移动到弃牌区。
--默认值为true。

--can_recast:
--布尔值，该牌能否被重铸。
--该值仅仅影响在鸡肋了该牌之后能否选中该牌，对于鸡肋了之后的各种效果，必须在about_to_use里进行判断。
--默认值为false，也就是不允许重铸。

--handling_method:
--Card::HandlingMethod枚举类型。
--此值的含义具体表现为牌的使用方法。由于这个值比较抽象所以不是很容易解释明白的。
--默认值：如果will_throw为true，则为sgs.Card_MethodDiscard，否则为sgs.Card_MethodUse。如果是仁德等等的技能牌，请将此值设置为sgs.Card_MethodNone。

--mute：
--布尔值，该牌使用时是否不播放声音。
--技能牌在使用时默认会自动播放对应与该技能牌的声音，如果将此值设置为true，则不会播放声音。
--默认值为false。

--filter：
--lua函数，返回一个布尔值，类似于ViewAsSkill中的view_filter，但filter方法的对象是玩家目标。
--你在使用牌时只能指定玩家为对象，不能直接指定玩家的某张牌为对象；
--比如过河拆桥，在神杀中，“选择对方的一张牌并弃置”是过河拆桥的效果，但过河拆桥的对象只有对方玩家。
--如果你确实需要“作为对象的牌”，请还是在了解游戏机制后自行发明解决方法……
--传入参数为self,targets(已经选择的玩家),to_select(需要判断是否能选择的玩家)
--默认条件为“一名其他玩家”。

--feasible：
--lua函数，返回一个布尔值，相当于viewasSkill的view_as方法是否应该返回nil。
--在viewAsSkill中，我们可以无数次选中牌，直到返回了有意义的view_as再点确定，
--所以view_as返回了无意义的Nil也无所谓；然而在SkillCard当中，点确定的机会只有一次，
--因此我们规定用feasible来排除无效使用的情况。
--只有在feasible返回了true时，你才可以点确定。
--传入参数为self,targets(已经选择的玩家)
--默认条件为"target_fixed为true(不需要指定目标)，或者选择了至少一名玩家为目标"

--about_to_use:
--lua函数，无返回值，执行使用流程。
--传入参数为self, room（房间对象）, cardUse（卡牌使用结构体）
--无默认值，即调用系统默认的onUse函数，在默认的函数当中，会触发CardUsed事件，在此事件中调用on_use函数。

--on_use:
--lua函数，无返回值，类似于on_trigger，执行使用过程。
--传入参数为self,room(游戏房间对象),source(使用者),targets(牌的使用目标)
--无默认值，即调用系统默认的use函数，在默认的函数当中，会对每一名targets执行on_effect，最后如果该牌在PlaceTable，则将这张牌置入弃牌堆。

--on_effect：
--lua函数，无返回值，同样执行使用效果，但只定义对于某一个目标的效果。
--传入参数为self, effect
--通常情况下你只需要写on_effect或者on_use当中的一个。
--如果是没有目标或者是目标特定的技能，使用on_use；
--如果是有几个目标执行相同或类似的效果，使用on_effect。
--如果是玩家指定的目标，还是使用on_effect。

--on_validate:
--lua函数，返回值为const Card *类型，可以修改将要使用的牌和使用目标。
--传入参数为self, cardUse
--这个函数在源码当中优先于about_to_use调用。
--修改cardUse.card没有任何用途，但是修改cardUse.to是有用的。
--这个函数如果不返回self，则不会调用本技能卡的其余所有函数，而会将cardUse.card修改为本函数的返回值重新使用。
--这个函数被用于源码奇策/蛊惑中，而这两个技能都是使用dialog做的
--这个函数被用于源码傲才中，而傲才需要把获得的卡牌返回摸牌堆，这是LUA无法实现的
--这个函数被用于激将
--默认值：返回self，也就是不改变卡牌

--on_validate_in_response:
--lua函数，返回值为const Card *类型，可以修改将要响应的牌。
--传入参数为self, user
--这个函数在源码当中优先于CardResponded时机
--这个函数如果不返回self，则视为使用或打出其他牌
--这个函数被用于源码蛊惑中，而这个技能是使用dialog做的
--这个函数被用于源码傲才中，而傲才需要把获得的卡牌返回摸牌堆，这是LUA无法实现的
--这个函数被用于如虎天翼包的诸葛恪急思
--默认值：返回self，也就是不改变卡牌


--以下为“离间牌”的 feasible 以及filter方法：

filter = function(self, targets, to_select)
	if not to_select:isMale() then return false end
	if #targets == 0 then
		return true
	elseif #targets == 1 then
		local duel = newDuel()
		if to_select:isProhibited(targets[1], duel, targets[1]:getSiblings()) then return false end
		if to_select:isCardLimited(duel, sgs.Card_MethodUse) then return false end
		return true
	elseif #targets == 2 then
		return false
	end
end ,
feasible = function(self, targets)
	return #targets == 2
end ,

--on_use 和 on_effect 为牌的效果执行。他们的区别在于生效时机的不同。
--on_use在牌被使用时生效，而on_effect在牌在使用中对某一名目标进行结算时生效。
--因此，在不存在需要结算“一名玩家对另一名指定的玩家”的效果时，使用on_use实行效果即可；
--存在指定的目标时，则原则上应该使用on_effect。

--about_to_use、on_use和on_effect可以同时存在。
--如果about_to_use里没有调用thread:trigger(sgs.CardUsed, room, cardUse.from, data)这个函数，则on_use不起作用。
--如果on_use没有调用room:cardEffect(effect)函数，则on_effect不起作用。

--以下为“雷击牌”的on_effect方法：

on_effect=function(self,effect)

	--effect 为一个CardEffectStruct，其from和to为player对象，代表谁使用的牌对谁结算
	local from=effect.from
	local to  =effect.to
	local room=to:getRoom()
	
	--sefEmotion在玩家的头像上显示表情
	room:setEmotion(to,"bad")
	
	--进行判定时，首先创建“判定”对象。
	--pattern为一个只有3段的ExpPattern，由"|"隔开的三段分别匹配牌名、花色和点数	
	--good的定义和之后的判定结果获取有关。
	--原则上，之前的pattern与判定牌匹配时，如果这种情况下执行的效果对于判定者来说是“好”的，
	--那么good应该是true。
	local judge=sgs.JudgeStruct()
	judge.pattern = ".|spade"
	judge.good=false
	judge.reason="leiji"
	judge.who=to
	
	--然后，让room根据此判定对象进行判定。判定结果依然在judge里面。
	room:judge(judge)
	
	--如果判定结果是一个“坏”结果，那么造成伤害
	if judge.isBad() then
		--和判定一样，造成伤害时先创建伤害struct,然后交由room:damage执行
		local damage=sgs.DamageStruct()
        damage.card = nil
        damage.damage = 2
        damage.from = from
        damage.to = to
        damage.nature = sgs.DamageStruct_Thunder
		
		room:damage(damage)
	else
		room:setEmotion(from,"bad")
	end
	
end,

--以下为孙权“制衡”的on_use方法：

on_use=function(self,room,source,targets)
	--self代表技能牌本身。由于是将“任意张牌当成制衡牌使用”，
	--因此弃置制衡牌就等于弃置所有用来发动制衡的牌，也即被制衡掉的牌。
	room:throwCard(self)	

	--摸取相当于被用来发动制衡的牌的数目的牌。
	--可以用self:getSubcards()来获取这些牌的QList。
	room:drawCards(source,self:getSubcards():length())
end,

--以下为刘备“激将”的on_validate方法：

on_validate = function(self, cardUse)
	cardUse.m_isOwnerUse = false --设置这张牌不是卡牌所有者使用的
	local liubei = cardUse.from
	local targets = cardUse.to
	local room = liubei:getRoom() --注意此函数为服务器端调用，所以可以getRoom
	local slash = nil
	local lieges = room:getLieges("shu", liubei)
	for _, target in sgs.qlist(targets) do
		target:setFlags("LuaJijiangTarget") --其实这个是为了AI
	end
	for _, liege in sgs.qlist(lieges) do
		slash = room:askForCard(liege, "slash", "@jijiang-slash:" .. liubei:objectName(), sgs.QVariant(), sgs.Card_MethodResponse, liubei) --这是610的写法，现在要考虑夏侯氏的卡牌问题
		if slash then
			for _, target in sgs.qlist(targets) do
				target:setFlags("-LuaJijiangTarget")
			end
			return slash --返回激将者打出的杀
		end
	end
	for _, target in sgs.qlist(targets) do
		target:setFlags("-LuaJijiangTarget")
	end
	room:setPlayerFlag(liubei, "Global_LuaJijiangFailed")
	return nil --返回空值，否则如果不返回的话，将会执行on_use等一系列函数
end

--以下为如虎天翼诸葛恪“急思”的on_validate_in_response方法：
--急思：每名其他角色的回合限一次。当你需要使用【无懈可击】时，你可以与当前回合角色拼点，若你赢，你视为使用一张【无懈可击】。

on_validate_in_response = function(self, user)
	local room = user:getRoom()
	room:setPlayerMark(user, "jisiused", 1) --由于卡牌因on_validate或on_validate_in_response而发生变化时不会有history，所以在这里加一条
	local current = room:getCurrent() --获取当前回合角色，注意此值可能为空也可能无意义
	
	if (current == nil) or (current:isDead()) or (current:getPhase() == sgs.Player_NotActive) or (current:objectName() == user:objectName()) then
		return nil
	end
	
	room:broadcastSkillInvoke("jisi", 1)
	
	if user:pindian(current, "jisi") then --ServerPlayer::pindian 函数，执行拼点
		local nul = sgs.Sanguosha:cloneCard("nullification", sgs.Card_NoSuit, 0) --克隆一张无懈可击
		nul:setSkillName("_jisi")
		return nul --返回这张无懈可击
	else
		room:broadcastSkillInvoke("jisi", 4)
	end
	
	return nil --返回空，表示什么都没用
end

--附：源码急思技能
--[[

//tigerfly.h
class JisiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JisiCard();
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};



//tigerfly.cpp
JisiCard::JisiCard(){
    target_fixed = true;
    mute = true;
}

const Card *JisiCard::validateInResponse(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->setPlayerMark(player, "jisiused", 1);
    ServerPlayer *current = room->getCurrent();
    if (!current || current->isDead() || current->getPhase() == Player::NotActive || current == player)
        return NULL;

    room->broadcastSkillInvoke("jisi", 1);

    if (player->pindian(current, "jisi")){
        Nullification *nul = new Nullification(Card::NoSuit, 0);
        nul->setSkillName("_jisi");
        return nul;
    }
    else
        room->broadcastSkillInvoke("jisi", 4);

    return NULL;
}

class JisiVS: public ZeroCardViewAsSkill{
public:
    JisiVS(): ZeroCardViewAsSkill("jisi"){
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getMark("jisiused") == 0 && pattern == "nullification" && player->getPhase() == Player::NotActive;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return (player->getMark("jisiused") == 0 && !player->isKongcheng() && player->getPhase() == Player::NotActive);
    }

    virtual const Card *viewAs() const{
        return new JisiCard;
    }
};

class Jisi: public PhaseChangeSkill{
public:
    Jisi(): PhaseChangeSkill("jisi"){
        view_as_skill = new JisiVS;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (!target->getPhase() != Player::RoundStart)
            return false;

        Room *room = target->getRoom();
        QList<ServerPlayer *> zhugekes = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhugeke, zhugekes){
            if (zhugeke->getMark("jisiused") != 0)
                room->setPlayerMark(zhugeke, "jisiused", 0);
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 2 + 2;
    }
};

]]
