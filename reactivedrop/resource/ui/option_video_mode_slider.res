"resource/ui/option_video_mode_slider.res"
{
	"InteractiveArea"
	{
		"ControlName"		"Panel"
		"fieldName"			"InteractiveArea"
		"xpos"				"120"	[!$WIN32WIDE]
		"xpos"				"140"	[$WIN32WIDE]
		"ypos"				"0"
		"wide"				"120"	[!$WIN32WIDE]
		"wide"				"130"	[$WIN32WIDE]
		"tall"				"12"
		"navLeft"			"TextEntry"
	}

	"LblFieldName"
	{
		"ControlName"		"Label"
		"fieldName"			"LblFieldName"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"90"	[!$WIN32WIDE]
		"wide"				"110"	[$WIN32WIDE]
		"tall"				"12"
		"fgcolor_override"	"192 192 192 255"
	}

	"LblHint"
	{
		"ControlName"		"Label"
		"fieldName"			"LblHint"
		"xpos"				"0"
		"ypos"				"12"
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"font"				"DefaultVerySmall"
		"textAlignment"		"north-west"
		"wrap"				"1"
		"fgcolor_override"	"128 128 128 255"
	}

	"TextEntry"
	{
		"ControlName"		"TextEntry"
		"fieldName"			"TextEntry"
		"xpos"				"90"	[!$WIN32WIDE]
		"xpos"				"110"	[$WIN32WIDE]
		"ypos"				"0"
		"wide"				"30"
		"tall"				"12"
		"navRight"			"InteractiveArea"
		"bgcolor_override"	"32 32 32 255"
	}
}
