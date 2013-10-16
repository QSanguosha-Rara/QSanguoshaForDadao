module("extensions.testinvoke", package.seeall)
extension = sgs.Package("testinvoke")

tttestinvokecard = sgs.CreateSkillCard{
	name = "tttestinvoke" ,
	target_fixed = true,
	on_use = function(self, room, source)
		
		local jsonvalue = sgs.JsonArrayForLUA()
		jsonvalue:setNumberAt(0, 2)
		jsonvalue:setStringAt(1, "$HunziAnimate")
		jsonvalue:setStringAt(2, "5000")
		room:doBroadcastNotify(sgs.CommandType.S_COMMAND_ANIMATE, jsonvalue)
		
		local jsonvalue2 = sgs.JsonArrayForLUA()
		jsonvalue2:setNumberAt(0, 2)
		jsonvalue2:setStringAt(1, "hunzi")
		jsonvalue2:setBoolAt(2, true)
		jsonvalue2:setNumberAt(3, -1)
		room:doBroadcastNotify(sgs.CommandType.S_COMMAND_LOG_EVENT, jsonvalue2)
		
		room:getThread():delay(5000)
	end
}

tttestinvoke = sgs.CreateViewAsSkill{
	name = "tttestinvoke" ,
	n = 0 ,
	view_as = function()
		return tttestinvokecard:clone()
	end ,
}

ttest = sgs.General(extension, "tttest", "wei")
ttest:addSkill(tttestinvoke)

sgs.LoadTranslationTable{
	["tttestinvoke"] = "魂姿特效",
}
