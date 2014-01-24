--技能详�：TriggerSkill 触发技

--许多技能都有”在xx时机，如果xx条件满足，那么执行xx效果“这样的描述�
--由于游戏技时机繁多，许多技能也都是相互类似的，我们在游戏的cpp部分有种类繁多的触发技定义方式�
--而基本的（也是万能的）触发技定义在Lua中是使用sgs.CreateTriggerSkill方法，该方法可以在lua\sgs_ex.lua中找到�

--CreateTriggerSkill需要以下参数：

--name, frequency, limit_mark, events, global, on_trigger, can_trigger, priority

--name�
--技能名称字符串
--没有默认……技能就是要有名字才�

--frequency�
--Frequency枚举类型，技能的发动频率�
--执行askForSkillInvoke（询问技能发动）时，frequency会影响玩家做决定的方式�
--frequency也起到了技能分类以及用于增加技能提示显示的作用�
	--frequency可能的值有�
	--sgs.Skill_Frequent（频繁发动：该技能会有一个可以打钩的按钮，如果勾选上，askForSkillInvoke就不会弹出提示而是直接返回true�
	--sgs.Skill_NotFrequent（不频繁发动：该技能的askForSkillInvoke总是会弹出提示询问玩家是否发动）
	--sgs.Skill_Compulsory （锁定技：该技能的默认优先度为2而不�；该技能会在显示上提示玩家这是一个锁定技能）
	--sgs.Skill_Limited （限定技：该技能会在显示上提示玩家这是一个限定技能）
	--ssg.Skill_Wake（觉醒技：该技能的默认优先度为2而不�；该技能会在显示上提示玩家这是一个觉醒技�
--frequency的默认值为sgs.Skill_NotFrequent

--limit_mark:
--字符串，如果此技能为限定技，则在游戏开始时会自动赋予以此值命名的mark�
--无默认值�

--events�
--Event枚举类型，或者一个包含Event枚举类型的lua表。代表该技能的触发时机�
--可用的Event列表请参考游戏代码中的struct.h文件�
--无默认值�

--global:
--布尔值。指示此技能是否为全局触发技�
--全局触发技加入技能表之后，即使没有人拥有这个技能，在游戏执行中只要满足can_trigger也会触发�
--默认值为false，即此技能不是全局触发技�

--on_trigger:
--lua函数，无返回值，执行事件触发时的技能效果�
--如果需要区分不同的事件执行不同效果，请根据event参数使用条件语句�
--通常需要将事件数据(data)转为具体的游戏结构对象才能进行操作。你可以在源码的swig/qvariant.i文件中看到定义�
--on_trigger的传入参数为self(技能对象本�,event(当前触发的事�,player(事件触发�,data(事件数据)
--无默认值�

--can_trigger:
--lua函数，返回一个布尔值，即是否能触发该技能�
--传入参数为self(技能对�,target(事件触发�
--默认条件为“具有本技能并且存活�
--在这里个人只建议写简单的条件，许多判断都放在on_trigger里面做return其实都是可以�

--priority:
--整数值，代表本技能的优先度�
--如果本技能与其他技能（或规则）在同一个时机都触发，那么优先度影响这些技能或规则的执行顺序�
--优先度更大的技能（或规则）优先执行。游戏规则的优先度为0，典型的技能优先度�，而受到伤害发动的技能优先度通常�1.
--锁定技和觉醒技的优先度默认�，其他情况下默认�

-- **实例�

--以下是曹操奸雄的实现�

Jianxiong = sgs.CreateTriggerSkill{
	name = "Jianxiong",
	events = {sgs.Damaged}, --或者events=sgs.Damaged
	on_trigger = function(self, event, player, data)
		local room = player:getRoom()
		local damage = data:toDamage()
		--这两步通常是必要的。我们需要room对象来操作大多数的游戏要素，我们也需要将data对象转成对应的数据类型来得到相应的信息�
		local card = damage.card
		if card then
			local id = card:getEffectiveId()
			if room:getCardPlace(id) == sgs.Player_PlaceTable then
				local card_data = sgs.QVariant()
				card_data:setValue(card)
				if room:askForSkillInvoke(player, self:objectName(), card_data) then
					player:obtainCard(card);
				end
			end
		end
	end
}

--在on_trigger方法中，我们首先获取了room对象�

--对于影响整盘游戏的效果，我们必须需要获取room对象。大多数情况下，room对象都是必须获取的�

--on_trigger方法的data参数是一个QVariant，根据不同的事件我们需要用不同的方法得到它原本的数据类型�
--对于Damaged事件（你受到了伤害），data对象的类型是DamageStruct，我们使用toDamage()得到DamageStruct�

--询问技能发动时，需要使用room对象的askForSkillInvoke方法�
--broadcastSkillInvoke方法则可以播放技能的发动效果。（但是对技能发动效果本身没有影响）

--player:obtainCard(card) 即让player得到造成伤害的card�

--在”某个阶段可触发“的技能，或者”摸牌时改为xx“这样的技能，可以使用EventPhaseStart事件来触发，并对event对象进行判断进行触发控制�

--对于在复数个时机发动的触发技，我们需要在on_trigger中使用条件语句�

--以下是袁术”庸肆“技能的实现�

yongsi=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_Compulsory, --锁定技
	
	name      = "yongsi",
	
	events={sgs.DrawNCards,sgs.EventPhaseStart}, --两个触发时机
	
	on_trigger=function(self,event,player,data)
	
		local room=player:getRoom()
		
		local getKingdoms=function() --可以在函数中定义函数，本函数返回存活势力的数�
			local kingdoms={}
			local kingdom_number=0
			local players=room:getAlivePlayers()
			for _,aplayer in sgs.qlist(players) do
				if not kingdoms[aplayer:getKingdom()] then
					kingdoms[aplayer:getKingdom()]=true
					kingdom_number=kingdom_number+1
				end
			end
			return kingdom_number
		end
		
		if event==sgs.DrawNCards then 
			--摸牌阶段，改变摸牌数
			room:broadcastSkillInvoke("yongsi")
			data:setValue(data:toInt()+getKingdoms()) 
			--DrawNCards事件的data是一个int类型的QVariant即摸牌数，改变该QVariant对象会改变摸牌数
		elseif (event==sgs.EventPhaseStart) and (player:getPhase()==sgs.Player_Discard) then
			--进入弃牌阶段时，先执行庸肆弃牌，然后再执行常规弃�
			local x = getKingdoms()
			local e = player:getEquips():length()+player:getHandcardNum()
			if e>x then room:askForDiscard(player,"yongsi",x,false,true) 
			--要求玩家弃掉一些牌
			-- 最后两个参数为”是否强制“以及”是否包含装备�
			else 
			--如果玩家的牌未达到庸肆的弃牌数目，那么跳过询问全部弃�
				player:throwAllHandCards()
				player:throwAllEquips()
			end
		end
	end
}