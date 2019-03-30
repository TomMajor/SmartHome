/**
 * @class
 **/

HbStatusDisplayDialogEPaper = Class.create(StatusDisplayDialog, {

  initEPaper: function () {
    conInfo("HbStatusDisplayDialogEPaper - initEPaper");
    this.displayType = "DIS-EP42BW";
  },

    // This creates the content of the dialog.
  _addElements: function() {
    var dialogContentElem = jQuery("#statusDisplayDialog"),
    textOptions = this._getTextOptions(),
    iconOptions = this._getIconOptions(),

    freeTextValue = translateKey("statusDisplayOptionFreeText"),
    arrDisabledElements = ["disabled", "","","","disabled","disabled"];

    dialogContentElem.append(function(index,html){
      //var content =  "<tr><th>&nbsp;</th><th>Text</th> <th id='headFreeText' class='hidden'></th><th name='optionHeader'>Color</th><th name='optionHeader'>Icon</th></tr>";
      var content =  "<tr><th>&nbsp;</th><th>Text</th> <th id='headFreeText' class='hidden'></th><th name='optionHeader'>Icon</th></tr>";
      for (var loop = 0; loop <= 9; loop++) {
        content +=
          "<tr>" +
            "<td>"+translateKey("statusDisplayLine")+ " "+(loop + 1)+": </td>"+
            "<td><select id='textSelect_"+loop+"' onchange='textOnChange(this)' >"+textOptions+"</select></td>" +
            "<td id='cellFreeText_"+loop+"' class='hidden'><input id='freeText_"+loop+"' type='text' value='"+freeTextValue+"' maxlength='16' size='19' style='text-align:center'></td>" +
            "<td id='placeHolder_"+loop+"' class='hidden'></td>" +
            "<td name='optionContainer_"+loop+"' class='hidden'><select id='iconSelect_"+loop+"' onchange='iconOnChange(this)'>"+iconOptions+"</select></td>" +
            "<td name='optionContainer_"+loop+"' class='hidden' id='iconPreview_"+loop+"'></td>"+
          "</tr>";
      }

      content += "<tr><td colspan='4'><hr></td></tr>";

      content += "<tr><td height='15px;'></td></tr>";

      return content;
    });
  },

    // Creates the options for the text selector
  _getTextOptions: function() {
    var options = "";
    options += "<option value='-1'>"+translateKey("stringTableNotUsed")+"</option>";
    for (var loop = 0; loop <= 19; loop++) {
      options += "<option value='"+loop+"'>"+ translateKey("statusDisplayOptionText")+ " " +(loop + 1)+"</option>";
    }
    options += "<option value='99'>"+translateKey("statusDisplayOptionFreeText")+"</option>";
    return options;
  },

    // Creates the options for the icon selector
  _getIconOptions: function() {
    var options = "",
    arOptionText = [
      translateKey("iconOff"),
      translateKey("iconOn"),
      translateKey("iconOpen"),
      translateKey("iconClosed"),
      translateKey("iconError"),
      translateKey("iconOK"),
      translateKey("iconInfo"),
      translateKey("iconNewMessage"),
      translateKey("iconServiceMessage"),
      translateKey("iconHbGarage"),
      translateKey("iconHbShutter"),
      translateKey("iconHbShutterUp"),
      translateKey("iconHbShutterDown"),
      translateKey("iconHbUp"),
      translateKey("iconHbDown"),
      translateKey("iconHbSnowflake"),
      translateKey("iconHbWarning"),
      translateKey("iconHbError"),
      translateKey("iconHbBell"),
      translateKey("iconHbCalendar"),
      translateKey("iconHbThermometer")
    ];

    options += "<option name='option_NotUsed' value='-1'>" + translateKey("stringTableNotUsed") + "</option>";
    for (var loop = 0; loop < 21; loop++) {
      options += "<option name='option_"+loop+"' value='"+loop+"'>" + arOptionText[loop] + "</option>";
    }
    return options;
  },

   // Creates the string, necessary for the text field within the program
  _createConfigString: function() {
    var textElm, freeTextElm,iconElm,
    result = this.startKey + ","; // Start key

    // Read 10 lines and create string
    for (var loop = 0; loop < 10; loop++) {
      textElm = jQuery("#textSelect_" + loop);
      iconElm = jQuery("#iconSelect_" + loop);
      freeTextElm = jQuery("#freeText_" + loop);

      if (textElm.val() == -1) {
        result+= this.lf + ",";
      } else {
        result += this.textKey + ",";
        if (textElm.val() != "99") {
          // Predefined text bloc
          result += this._convertVal2HexVal(textElm.val()) + ",";
        } else {
          // Free user customized text
          //result += freeTextElm.val() + ",";
          result += this._convertPlainText2Hex(freeTextElm.val());
        }
        if (iconElm.val() != -1) {
          result += this.iconKey + ",";
          result += this._convertVal2HexVal(iconElm.val()) + ",";
        }
        result += this.lf + ",";
      }
    }
    result+= this.endKey; // End key
    this.configString = result;
  },
  /**
   * Initializes the dialog
   * @private
   */
  _initAllValues: function() {
    var self = this,
    arValues = this.channelValue.split(","),
    sizeChannelValue = arValues.length,
    arAllValues = this._getAllValues();

    conInfo("DIS-EP42BW: All values of the channel: ");
    conInfo(arAllValues);

    jQuery.each(arAllValues, function(index, line) {
      var textElm = jQuery("#textSelect_" + index),
      iconElm = jQuery("#iconSelect_" + index),
      freeTextElm = jQuery("#freeText_" + index);

      if (line.text != "notUsed") {
        textElm.val(self._convertHexVal2Val(line.text));
        // User defined text
        if (parseInt(line.text.split(",")[0],16) < 128) {
          freeTextElm.val(decodeStringStatusDisplay(self._convertHexString2PlainText(line.text)));
        }
        if (line.icon != -1) {
          iconElm.val(self._convertHexVal2Val(line.icon));
          setIconPreview({index: index, value: iconElm.val()});
        } else {
          // Icon not in use
          iconElm.val("-1");
        }
        displayStatusDisplayOptionContainer(index, true);
      } else {
        textElm.val("-1");
      }
    });

    setFreeTextContainer();
  },
    // Returns an array of objects with the values of all lines.
  // [Object {text="0x80, color="0x81", icon="0x82},.....]}
  _getAllValues: function() {
    var val = this.channelValue;
    var arValues = val.split(","), //replace(/ /g, "").split(","),
    arLines = []; // contains the lines 1,2,3

    // Is a start key and end key available? Otherwise the string isn¬•t valid.
    if (arValues[0] == this.startKey && arValues[arValues.length - 1] == this.endKey) {
      var lineIndex = 0,
      textIndex,
      nextTextBlockIndex = 0,
      textOffset = 0;

      arValues.shift(); // remove the start key 0x02
      arValues.pop(); // remove the end key 0x03
      //console.log("arValues: " + arValues);
      for (var loopx = 0; loopx < arValues.length; loopx++) {
        //console.log("current loopx: " + loopx);
        var valueSet = {};
        nextTextBlockIndex = 0;
                // Is LF?
        if (arValues[loopx] == this.lf) {
          valueSet.text="notUsed";
           arLines[lineIndex] = valueSet;
          lineIndex++;
        }

        if (arValues[loopx] == this.textKey) {
          valueSet.text = "";
          // Read till icon or lf and increase loopx by the number of read chars
          textIndex = loopx + 1;
          do {
            valueSet.text += arValues[textIndex];
            if ((arValues[textIndex + 1] != this.iconKey) && (arValues[textIndex + 1] != this.lf)) {
              valueSet.text += ",";
            }

            //console.log("added char: " + arValues[textIndex]);
            textIndex++;
            nextTextBlockIndex++;
          } while ((arValues[textIndex] != this.iconKey) && (arValues[textIndex] != this.lf)) ;

          // Icon hinzuf¬∏gen, entweder nicht benutzt (-1) oder den entsprechenden Wert
          // Add the icon, either not used (-1) or the correspondent value
          valueSet.icon = (arValues[textIndex] == this.iconKey) ? arValues[textIndex + 1] : -1;
          arLines[lineIndex] = valueSet;
          lineIndex++;
          if (valueSet.icon == -1) {textOffset = 1;} else {textOffset = 3;}
          // Jump to the next text block
          loopx += nextTextBlockIndex + textOffset; // Springe zum n‚Ä∞chsten Textblock
          //console.log("new loopx : " + loopx);
        }
      }
    } else {
      conInfo("Value string invalid");
    }
    return arLines;
  }
});

YesNoDialog.RESULT_NO = 0;
YesNoDialog.RESULT_YES = 1;
