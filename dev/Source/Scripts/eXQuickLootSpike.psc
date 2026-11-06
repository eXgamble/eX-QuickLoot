Scriptname eXQuickLootSpike Hidden
{Dev-only test natives for the perk-option spike (SKY-66). Results go to QuickLootIE.log.}

; Logs the activate-choice perk options (Feed, Harvest Blood, ...) the player has on akTarget.
Function ListOptions(ObjectReference akTarget) global native

; Runs option aiIndex of that list by activating akTarget and answering the game's own choice popup.
Function RunIndex(ObjectReference akTarget, Int aiIndex) global native

; Activates akTarget and answers the choice popup with the default (Search).
Function SearchTarget(ObjectReference akTarget) global native
