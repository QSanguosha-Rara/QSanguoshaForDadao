--根据V1的基础上
function SmartAI:useCardShit(card, use)
	if (card:getSuit() == sgs.Card_Heart or card:getSuit() == sgs.Card_Club) and self.player:isChained() and
		#(self:getChainedFriends()) > #(self:getChainedEnemies()) then return end
	if self.player:getHp()>3 and (self.player:hasSkills("shenfen+kuangbao") or self:getDamagedEffects()) then use.card = card return end
	if self.player:hasSkill("kuanggu") and card:getSuitString() ~= "spade" then use.card = card return end
	if card:getSuit() == sgs.Card_Heart and (self.player:hasArmorEffect("GaleShell") or self.player:hasArmorEffect("Vine")) then return end
	if not self.player:isWounded() then
		if self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1 then use.card = card return end
		for _, askill in sgs.qlist(self.player:getVisibleSkillList()) do
			if sgs[askill:objectName() .. "_suit_value"] then
				if (sgs[askill:objectName() .. "_suit_value"][card:getSuitString()] or 0) > 0 then return end
			end
		end
		local peach = self:getCard("Peach")
		if peach then
			self:sort(self.friends, "hp")
			if not self:isWeak(self.friends[1]) then
				use.card = card
				return
			end
		end
	end
end

sgs.ai_use_value.Shit = -10
sgs.ai_keep_value.Shit = 6
