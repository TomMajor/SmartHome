#!/bin/tclsh
#
# =================================================
# epaper42.tcl
# HB-Dis-EP-42BW script helper
# Version 0.52
# 2019-10-02 Tom Major (Creative Commons)
# https://creativecommons.org/licenses/by-nc-sa/4.0/
# You are free to Share & Adapt under the following terms:
# Give Credit, NonCommercial, ShareAlike
#
# usage: epaper42 serial /line text [icon number] [/nextline text [icon number]] ...
#
# * Der erste Parameter ist die Seriennummer des Displays, z.B. JPDISEP000
# * Jede neue Zeile beginnt mit einem / gefolgt von der Zeilennummer.
# * Danach folgt der anzuzeigende Text, enthält der Text Leerzeichen muss man den ganzen Text in '' einschliessen, andernfalls geht es auch ohne.
# * Die im WebUI vordefinierten Texte 1..20 werden mit dem Code @txx erzeugt, wobei xx zwischen 01 und 20 liegen kann und immer 2 Stellen haben muss.
# * Der 3. Parameter ist die Iconnummer oder einfach den Parameter weglassen wenn man kein Icon in der Zeile haben will.
#     Die Iconnummer braucht nur eine Stelle bei Icons < 10.
# * CUxD/CMD_EXEC wie in den Bsp. braucht man dabei nicht zwingend. Man kann das auch mit system.Exec() aufrufen.
# * Ab Version 0.50 kann man für Texte die x-Position angeben um z.B. eine Darstellung in Spalten zu erreichen.
#     Dies geht mit @pxx, xx gibt hier die Anfangsposition des Textes in % der Displaybreite an.
#     Das gleiche bewirkt @fxx, nur dann in Farbe falls es ein (3-)Farben-Display ist.
#     Das Feature x-Position ist an 2 Bedingungen geknüpft:
#     1) Die Textzeile muss in den Geräteeinstellungen auf 'linksbündig' eingestellt sein.
#     2) Der Text muss mit einem solchen x-Positionscode @pxx (oder @fxx) anfangen um den Textanfang eindeutig zu bestimmen.
#     Falls der Text x-Positionscodes enthält wird kein Icon angezeigt.
#
# Beispiel 1 - variabler Text in einer Zeile:
# Zeigt den Text 'Test ABC ÄÖÜäöüß' in Zeile 5 mit Icon 1 auf dem ePaper mit Serial JPDISEP000 an:
# string displayCmd = "JPDISEP000 /5 'Test ABC ÄÖÜäöüß' 1";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
# oder
# system.Exec("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 2 - variabler Text in mehreren Zeilen:
# Zeigt 3 Zeilen Text in den Zeilen 5, 7, 10 an, dabei Zeile 5 und 10 mit Icons, 7 ohne Icon
# string displayCmd = "JPDISEP000 /5 'Test ABC 123' 1 /7 Textzeile_7 /10 Textzeile_10 12";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 3 - Sensorwert
# Zeigt die Temperatur vom Gerät UNISENS077 in Zeile 2 an
# string temp = dom.GetObject('BidCos-RF.UNISENS077:1.TEMPERATURE').Value().ToString(1) # "°C";
# string displayCmd = "JPDISEP000 /2 'Temperatur " # temp # "'";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 4 - vordefinierte Texte
# Zeigt den vordef. Text 4 in Zeile 1, den vordef. Text 19 in Zeile 9 und den vordef. Text 20 in Zeile1 10 an, Zeile 1 zusätzlich mit Icon
# string displayCmd = "JPDISEP000 /1 @t04 6 /9 @t19 /10 @t20";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 5 - variable und vordefinierte Texte gemischt in einer Zeile
# Zeigt den vordef. Text 2 gemischt mit variablen Text in Zeile 1 an
# string displayCmd = "JPDISEP000 /1 abcd@t02efgh";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 6 - x-Position
# Zeigt den vordef. Text 9 (Außen) bei 3% x-Position, Temperatur bei 45% x-Position, Feuchte bei 78% x-Position an
# string tempOut = dom.GetObject('BidCos-RF.UNISENS077:1.TEMPERATURE').Value().ToString(1) # " °C";
# string humOut = dom.GetObject('BidCos-RF.UNISENS077:1.HUMIDITY').Value().ToString(0) # " %";
# string displayCmd = "JPDISEP000 /3 '@p03@t09@p45" # tempOut # "@p78" # humOut # "'";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 7.1 - Farbe zeilenweise
# Zeigt den Text in Zeile 2 in Normalfarbe, den Text in Zeile 3 in Rot
# string displayCmd = "JPDISEP000 /2 'Eine normale Zeile' /3 '@fzEine rote Zeile'";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 7.2 - Farbe mit x-Position / spaltenweise
# Zeigt den Text "Garage" in Zeile 2 und 3 bei 3% x-Position in Normalfarbe
# und bei 50% x-Position wird "Auf" in Rot und "Zu" in Normalfarbe dargestellt
# string displayCmd = "JPDISEP000 /2 '@p03Garage@f50Auf' /3 '@p03Garage@p50Zu'";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# =================================================

load tclrega.so

# -------------------------------------
proc main { argc argv } {
    
    if { $argc < 3 } {
        return
    }
    
    for { set i 0 }  { $i <= 10 }  { incr i } {
        set DISPLAY($i) ""
    }
    
    set SERIAL [lindex $argv 0]

    set i 0
    foreach subCmd [split $argv "/"] {
        if { $i > 0 && $i <= 10 } {
            set LINE [lindex $subCmd 0]
            set TEXT [lindex $subCmd 1]
            set ICON [lindex $subCmd 2]

            # process each display line
            if { $LINE >= 1 && $LINE <= 10 && [string length $TEXT] > 0 } {

                #debugLog "<$LINE><$TEXT><$ICON>"

                # test for color code @fz (line-wise), only for 3-color ePapers
                if { [string first "@fz" $TEXT] == 0 } {
                    set TEXT [string replace $TEXT 0 2]
                    set txtOut "0x16,"
                } else {
                    set txtOut "0x12,"
                }

                # fixed or variable text, can be combined in one line
                for { set n 0 } { $n < [string length $TEXT] } { incr n } {
                    set char [string index $TEXT $n]
                    set nextchar [string index $TEXT [expr $n + 1]]
                    scan $char "%c" numDec
                    set numHex [format %x $numDec]
                    # check for
                    # 1) fixed text code @txx, 2 digits required, or
                    # 2) special codes with @cxx syntax
                    # pass thru all other @yxx codes!
                    if { ($numDec == 64) && (($nextchar == "t") || ($nextchar == "c")) } {
                        set numberStr [string range $TEXT [expr $n + 2] [expr $n + 3]]
                        if { [string length $numberStr] == 2 } {
                            # this scan here is required to extract numbers like 08 or 09 correctly, otherwise the number is treated as octal which will result in errors
                            scan $numberStr "%d" number
                            if { ($nextchar == "t") && ($number >= 1) && ($number <= 20) } {
                                # @t01..@t20
                                set textDec [expr 127 + $number]
                                set textHex [format %x $textDec]
                                append txtOut "0x$textHex,"
                                incr n 3
                                continue
                            } elseif { ($nextchar == "c") && ($number == 0) } {
                                # @c00 -> "/"
                                append txtOut "0x2F,"
                                incr n 3
                                continue
                            }
                        }
                    }
                    # variable text, hex 30..5A, 61..7A
                    if { ($numDec >= 48 && $numDec <= 90) ||
                         ($numDec >= 97 && $numDec <= 122) } {
                        append txtOut "0x$numHex,"
                    } else {
                        append txtOut "0x[encodeSpecialChar $numHex],"
                    }
                }

                # optional icon
                if { $ICON >= 1 && $ICON <= 50 } {
                    append txtOut "0x13,"
                    set iconDec [expr 127 + $ICON]
                    set iconHex [format %x $iconDec]
                    append txtOut "0x$iconHex,"
                }
                
                # store line cmd
                set DISPLAY($LINE) $txtOut
            }
        }
        incr i
    }
    
    # build complete display command
    set displayCmd "0x02,"
    for { set i 1 } { $i <= 10 } { incr i } {
        if { [string length $DISPLAY($i)] > 0 } {
            append displayCmd $DISPLAY($i)
        } 
        # line end
        append displayCmd "0x0A,"
    }
    append displayCmd "0x03"
    #debugLog $displayCmd
    
    set rega_cmd ""
    append rega_cmd "dom.GetObject('BidCos-RF.$SERIAL:11.SUBMIT').State('$displayCmd');"
    array set regaRet [rega_script $rega_cmd]
}

# -------------------------------------
proc encodeSpecialChar { numHex } {
	switch $numHex {
    
        20      { return "20" }     # space
        21      { return "21" }     # !
        25      { return "25" }     # %
        27      { return "27" }     # =
        28      { return "28" }     # (
        29      { return "29" }     # )
        2a      { return "2A" }     # *
        2b      { return "2B" }     # +
        2c      { return "2C" }     # ,
        2d      { return "2D" }     # -
        2e      { return "2E" }     # .
        b0      { return "B0" }     # °
        5f      { return "5F" }     # _

        c4      { return "5B" }     # Ä
        d6      { return "23" }     # Ö
        dc      { return "24" }     # Ü
        e4      { return "7B" }     # ä
        f6      { return "7C" }     # ö
        fc      { return "7D" }     # ü
        df      { return "7E" }     # ß

    	default {
            #debugLog "Unknown: $numHex"
            return "2E" 
        }
	}
}

# -------------------------------------
proc debugLog { text } {
    #set fileId [open "/media/usb1/debug.log" "a+"]
	#puts $fileId $text
	#close $fileId
}

main $argc $argv
