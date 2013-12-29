--技能讲解4：基本技能类型的组合与复杂技能的实现

--大多数技能都不是单独使用ViewAsSkill或者TriggerSkill就可以实现的。复杂的技能往往需要结合多种基本技能。

--一个的技能的描述，往往可以分成3部分：发动的时机、发动代价、效果。

--对于发动时机的控制，是只有触发技可以实现的。触发技可以在任何“有定义的时机”时产生作用。然而，对于需要根据消耗牌来决定效果的那些技能，用触发技往往无法实现，也很难符合游戏流程。

--ViewAsSkill可以非常完美地实现技能的发动流程。只有VIewAsSkill可以细致地操纵玩家发动技能所付出的代价。然而，ViewAsSkill本身并没有技能效果的实现，只能使用游戏牌作为技能效果，或者借助SkillCard。

--SkillCard本身并不是一个技能，但是只有他可以把技能发动用牌和技能效果联系起来。它对于ViewAsSkill而言是必需的。


--1.结合基本技能的方法：

--我们把技能效果定义在一个技能牌skill_card当中，技能牌的定义可以不从属于任何技能。然后，由于时机判断是整个技能的发动最先进行的一步，我们将ViewAsSkill定义在TriggerSkill当中。最后，我们把TriggerSkill的发动效果设为“询问使用skill_card”，而ViewAsSkill的效果设为“你可以将特定牌作为skill_card使用或打出”。如此，即实现了“xxx时，你可以使用xx牌发动，执行xxx效果。”

--以下为张辽突袭技能的实现：


LuaTuxiCard = sgs.CreateSkillCard{
	name = "LuaTuxiCard",
	filter = function(self, targets, to_select)
		if (#targets >= 2) or (to_select:objectName() == sgs.Self:objectName()) then
			return false
		end
		return not to_select:isKongcheng()
	end,
	on_use = function(self, room, source, targets)
		local moves = sgs.CardsMoveList()
		local move1 = sgs.CardsMoveStruct()
		move1.card_ids:append(room:askForCardChosen(source, targets[1], "h", self:objectName()))
		move1.to = source
		move1.to_place = sgs.Player_PlaceHand
		moves:append(move1)
		if #targets == 2 then
			local move2 = sgs.CardsMoveStruct()
			move2.card_ids:append(room:askForCardChosen(source, targets[2], "h", self:objectName()))
			move2.to = source
			move2.to_place = sgs.Player_PlaceHand
			moves:append(move2)
		end
		room:moveCards(moves, false)
	end
}
LuaTuxiVS = sgs.CreateViewAsSkill{
	name = "LuaTuxi",
	n = 0,
	view_as = function(self, cards)
		return LuaTuxiCard:clone()
	end,
	enabled_at_play = function(self, player)
		return false
	end,
	enabled_at_response = function(self, player, pattern)
		return pattern == "@@LuaTuxi"
	end
}
LuaTuxi = sgs.CreateTriggerSkill{
	name = "LuaTuxi",
	frequency = sgs.Skill_NotFrequent,
	events = {sgs.EventPhaseStart},
	view_as_skill = LuaTuxiVS,
	on_trigger = function(self, event, player, data)
		if player:getPhase() == sgs.Player_Draw then
			local room = player:getRoom()
			local can_invoke = false
			local other_players = room:getOtherPlayers(player)
			for _,target in sgs.qlist(other_players) do
				if not target:isKongcheng() then
					can_invoke = true
					break;
				end
			end
			if can_invoke then
				if room:askForUseCard(player, "@@LuaTuxi", "@tuxi-card") then
					return true
				end
			end
		end
		return false
	end
}
sgs.LoadTranslationTable{
	["LuaTuxi"] = "lua突袭",
	[":LuaTuxi"] = "和突袭技能描述一摸一样",
	["@tuxi-card"] = "和突袭的询问字串一摸一样",
}
--翻译代码

--技能实现说明：
--此技能的实现分3个部分
--第一部分是突袭牌，效果为“从两名角色的手牌中各获得一张牌”
--第二部分是突袭ViewAsSkill，“每当你需要使用一张突袭牌时，你可以视为使用一张突袭牌”
--第三部分是突袭TriggerSkill，“触发技，摸牌阶段开始时，你可以使用一张突袭牌，若你在此阶段使用了突袭牌，则放弃摸牌”