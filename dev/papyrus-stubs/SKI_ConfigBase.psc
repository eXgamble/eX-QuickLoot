Scriptname SKI_ConfigBase extends SKI_QuestBase Hidden
; compile-only stub generated from the papyrus-reference SkyUI SDK signatures (eX QuickLoot release)

String Property CurrentPage Hidden
	String Function Get()
		return ""
	EndFunction
EndProperty
Int Property LEFT_TO_RIGHT = 1 AutoReadOnly
String Property ModName Auto
Int Property OPTION_FLAG_DISABLED = 0x1 AutoReadOnly
Int Property OPTION_FLAG_HIDDEN = 0x2 AutoReadOnly
Int Property OPTION_FLAG_NONE = 0x0 AutoReadOnly
Int Property OPTION_FLAG_WITH_UNMAP = 0x4 AutoReadOnly
String[] Property Pages Auto
Int Property TOP_TO_BOTTOM = 2 AutoReadOnly
Event OnColorAcceptST(Int a_color)
EndEvent
Event OnColorOpenST()
EndEvent
Event OnConfigClose()
EndEvent
Event OnConfigInit()
EndEvent
Event OnConfigOpen()
EndEvent
Event OnConfigRegister()
EndEvent
Event OnDefaultST()
EndEvent
Event OnHighlightST()
EndEvent
Event OnInputAcceptST(String a_input)
EndEvent
Event OnInputOpenST()
EndEvent
Event OnKeyMapChangeST(Int a_keyCode, String a_conflictControl, String a_conflictName)
EndEvent
Event OnMenuAcceptST(Int a_index)
EndEvent
Event OnMenuOpenST()
EndEvent
Event OnOptionColorAccept(Int a_option, Int a_color)
EndEvent
Event OnOptionColorOpen(Int a_option)
EndEvent
Event OnOptionDefault(Int a_option)
EndEvent
Event OnOptionHighlight(Int a_option)
EndEvent
Event OnOptionInputAccept(Int a_option, String a_input)
EndEvent
Event OnOptionInputOpen(Int a_option)
EndEvent
Event OnOptionKeyMapChange(Int a_option, Int a_keyCode, String a_conflictControl, String a_conflictName)
EndEvent
Event OnOptionMenuAccept(Int a_option, Int a_index)
EndEvent
Event OnOptionMenuOpen(Int a_option)
EndEvent
Event OnOptionSelect(Int a_option)
EndEvent
Event OnOptionSliderAccept(Int a_option, Float a_value)
EndEvent
Event OnOptionSliderOpen(Int a_option)
EndEvent
Event OnPageReset(String a_page)
EndEvent
Event OnSelectST()
EndEvent
Event OnSliderAcceptST(Float a_value)
EndEvent
Event OnSliderOpenST()
EndEvent
Event OnVersionUpdate(Int aVersion)
EndEvent
Int Function AddColorOption(String a_text, Int a_color, Int a_flags = 0)
	return 0
EndFunction
Function AddColorOptionST(String a_stateName, String a_text, Int a_color, Int a_flags = 0)
EndFunction
Int Function AddEmptyOption()
	return 0
EndFunction
Int Function AddHeaderOption(String a_text, Int a_flags = 0)
	return 0
EndFunction
Int Function AddInputOption(String a_text, String a_value, Int a_flags = 0)
	return 0
EndFunction
Function AddInputOptionST(String a_stateName, String a_text, String a_value, Int a_flags = 0)
EndFunction
Int Function AddKeyMapOption(String a_text, Int a_keyCode, Int a_flags = 0)
	return 0
EndFunction
Function AddKeyMapOptionST(String a_stateName, String a_text, Int a_keyCode, Int a_flags = 0)
EndFunction
Int Function AddMenuOption(String a_text, String a_value, Int a_flags = 0)
	return 0
EndFunction
Function AddMenuOptionST(String a_stateName, String a_text, String a_value, Int a_flags = 0)
EndFunction
Int Function AddSliderOption(String a_text, Float a_value, String a_formatString = "{0}", Int a_flags = 0)
	return 0
EndFunction
Function AddSliderOptionST(String a_stateName, String a_text, Float a_value, String a_formatString = "{0}", Int a_flags = 0)
EndFunction
Int Function AddTextOption(String a_text, String a_value, Int a_flags = 0)
	return 0
EndFunction
Function AddTextOptionST(String a_stateName, String a_text, String a_value, Int a_flags = 0)
EndFunction
Int Function AddToggleOption(String a_text, Bool a_checked, Int a_flags = 0)
	return 0
EndFunction
Function AddToggleOptionST(String a_stateName, String a_text, Bool a_checked, Int a_flags = 0)
EndFunction
Function ForcePageReset()
EndFunction
String Function GetCustomControl(Int a_keyCode)
	return ""
EndFunction
Int Function GetVersion()
	return 0
EndFunction
Function LoadCustomContent(String a_source, Float a_x = 0.0, Float a_y = 0.0)
EndFunction
Function SetColorDialogDefaultColor(Int a_color)
EndFunction
Function SetColorDialogStartColor(Int a_color)
EndFunction
Function SetColorOptionValue(Int a_option, Int a_color, Bool a_noUpdate = false)
EndFunction
Function SetColorOptionValueST(Int a_color, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetCursorFillMode(Int a_fillMode)
EndFunction
Function SetCursorPosition(Int a_position)
EndFunction
Function SetInfoText(String a_text)
EndFunction
Function SetInputDialogStartText(String a_text)
EndFunction
Function SetInputOptionValue(Int a_option, String a_value, Bool a_noUpdate = false)
EndFunction
Function SetInputOptionValueST(String a_value, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetKeyMapOptionValue(Int a_option, Int a_keyCode, Bool a_noUpdate = false)
EndFunction
Function SetKeyMapOptionValueST(Int a_keyCode, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetMenuDialogDefaultIndex(Int a_value)
EndFunction
Function SetMenuDialogOptions(String[] a_options)
EndFunction
Function SetMenuDialogStartIndex(Int a_value)
EndFunction
Function SetMenuOptionValue(Int a_option, String a_value, Bool a_noUpdate = false)
EndFunction
Function SetMenuOptionValueST(String a_value, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetOptionFlags(Int a_option, Int a_flags, Bool a_noUpdate = false)
EndFunction
Function SetOptionFlagsST(Int a_flags, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetSliderDialogDefaultValue(Float a_value)
EndFunction
Function SetSliderDialogInterval(Float a_value)
EndFunction
Function SetSliderDialogRange(Float a_minValue, Float a_maxValue)
EndFunction
Function SetSliderDialogStartValue(Float a_value)
EndFunction
Function SetSliderOptionValue(Int a_option, Float a_value, String a_formatString = "{0}", Bool a_noUpdate = false)
EndFunction
Function SetSliderOptionValueST(Float a_value, String a_formatString = "{0}", Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetTextOptionValue(Int a_option, String a_value, Bool a_noUpdate = false)
EndFunction
Function SetTextOptionValueST(String a_value, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Function SetTitleText(String a_text)
EndFunction
Function SetToggleOptionValue(Int a_option, Bool a_checked, Bool a_noUpdate = false)
EndFunction
Function SetToggleOptionValueST(Bool a_checked, Bool a_noUpdate = false, String a_stateName = "")
EndFunction
Bool Function ShowMessage(String a_message, Bool a_withCancel = true, String a_acceptLabel = "Accept", String a_cancelLabel = "Cancel")
	return false
EndFunction
Function UnloadCustomContent()
EndFunction
