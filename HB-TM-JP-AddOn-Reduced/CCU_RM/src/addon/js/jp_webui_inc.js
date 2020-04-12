/**
 * ise/iseButtonsServo.js
 **/

/**
 * @fileOverview ?
 * @author ise
 **/

/**
 * @class
 **/ 
iseButtonsServo = Class.create();

iseButtonsServo.prototype = {
  /*
   * id = DOM-ID of switch
   * initState = Creation State 
   */
  initialize: function(id, initState, lvlDP, oldLvlDP, iViewOnly, bSliderPosFlag, label)
  {
    conInfo( "iseServo: initialize()" );
    this.id = id;
    this.state = initState;
    this.lvlDP = lvlDP;
    this.oldLvlDP = oldLvlDP;

    if(bSliderPosFlag)
    {
        this.bSliderPosFlag = bSliderPosFlag;
    }
    else
    {
        this.bSliderPosFlag = false;
    }
    this.slider = new sliderControl("Servo", this.id, initState, iViewOnly,this.bSliderPosFlag, 0, 180, 0.555556, '&deg;');
    
    this.hasRampClicked = false;
    
    this.txtDeg = $(this.id + "Deg");
    
    // Add event handlers
    if (iViewOnly === 0)
    {
      this.mouseOut = this.onMouseOut.bindAsEventListener(this);
      Event.observe($("slidCtrl" + this.id), 'mouseout', this.mouseOut);
    
      this.rampClick = this.onRampClick.bindAsEventListener(this);
      Event.observe(this.slider.e_base, 'mousedown', this.rampClick);
      
      this.handleClick = this.onHandleClick.bindAsEventListener(this);
      Event.observe($("slidCtrl" + this.id), 'mouseup', this.handleClick);
      
      this.clickUp = this.onClickUp.bindAsEventListener(this);
      Event.observe($(this.id + "Up"), 'click', this.clickUp);

      this.clickDown = this.onClickDown.bindAsEventListener(this);
      Event.observe($(this.id + "Down"), 'click', this.clickDown);
      
      this.DegChange = this.onDegChange.bindAsEventListener(this);
      Event.observe($(this.id + "Deg"), 'change', this.DegChange);
    }
    this.refresh(false);
  },
  
  onMouseOut: function(event)
  {
    var e = event;
    if (!e) { e = window.event; }
    var relTarg = e.relatedTarget || e.fromElement;
    if( relTarg )
    {
      var b1 = (relTarg.id.indexOf("slider")!=-1);
      var b2 = (relTarg.id.indexOf("base")!=-1);
      var b3 = (relTarg.id.indexOf("green")!=-1);
      if( !b1 && !b2 && !b3 ) 
      {
        if( this.hasRampClicked )
        {
          conInfo( "iseServo: onMouseOut() ["+relTarg.id+"] "+this.slider.n_value  );
          this.hasRampClicked = false;
          this.state = this.slider.n_value;
          //this.refresh();
        }
      }
    }
  },
 
  onRampClick: function(ev)
  {
     conInfo( "iseServo: onRampClick()" );
     this.hasRampClicked = true;
     var pos = Position.page(this.slider.e_base);
     var offset = ev.clientX - pos[0];
     var val = ( offset * 180 ) / this.slider.n_controlWidth;  
     var oldstate = parseInt(this.state);
     this.state = Math.floor(val);
     if (this.state < (oldstate-3))
     {
       this.slider.f_setValue(val);     
     }     
     else if (this.state > (oldstate+3))
     {
       this.slider.f_setValue(val);     
     } 
     conInfo("setting Servo DP "+this.lvlDP+" State --> " + this.state + " -- old State --> "+oldstate);   
     //window.setTimeout("ibd"+this.id+".refresh()",1000);
  },
  
  onHandleClick: function()
  {
    conInfo( "iseServo: onHandleClick()" );
    //this.state = this.txtDeg.value;
    this.refresh();
  },
  
  onClickUp: function()
  {
    conInfo( "iseServo: onClickUp()" );
    this.state = this.slider.n_value;
    this.state += 10; 
    if (this.state > 180)
    {
      this.state = 180;
    }
    this.refresh();
  },
  
  onClickDown: function()
  {
    conInfo( "iseServo: onClickDown()" );
    this.state = this.slider.n_value;
    this.state -= 10; 
    if (this.state < 0)
      this.state = 0;
    this.refresh();
  },
   
  
  onDegChange: function()
  {
    conInfo( "iseServo: onDegChange()" );
    if( isNaN(this.txtDeg.value) ) return;
    if( parseInt(this.txtDeg.value) > 180 ) this.txtDeg.value = 180;
    if( parseInt(this.txtDeg.value) < 0 ) this.txtDeg.value = 0;
    this.state = this.txtDeg.value;
    this.refresh();
  },
  
  update: function(newVal)
  {
    conInfo( "iseServo: update()" );
    this.state = newVal;
    this.refresh(newVal);
  },
  
  refresh: function(setstate)
  {
    conInfo( "iseServo: refresh() " + this.state );
    this.slider.f_setValue(this.state, true);
    this.txtDeg.value = this.state;

    if(typeof setstate == "undefined")
    {
      conInfo("setting Servo DP "+this.lvlDP+" State -------> " + this.state);    
      setDpState(this.lvlDP, (this.state / 180));
    }
  }
};

/**
 * @class
 **/
iseRFIDKey = Class.create();

iseRFIDKey.prototype = {
  /*
   * id = datapoint-ID of switch
   */
  initialize: function(id, shortId, longId, iViewOnly) {
    this.id = id;
    this.divShort = $(this.id + "Short");
    this.divLong = $(this.id + "Long");
    this.shortId = shortId;
    this.longId = longId;
    
    if( this.divShort ) { ControlBtn.off(this.divShort); }
    if( this.divLong ) { ControlBtn.off(this.divLong); }
    
    // Add event handlers
    if (iViewOnly === 0)
    {
      if (this.divShort) {
        this.clickShort = this.onClickShort.bindAsEventListener(this);
        Event.observe(this.divShort, 'mousedown', this.clickShort);
      }
      if (this.divLong) {
        this.clickLong = this.onClickLong.bindAsEventListener(this);
        Event.observe(this.divLong, 'mousedown', this.clickLong);
      }
    }
  },
  
  onClickShort: function() {
    setDpState(this.shortId, 1);
    ControlBtn.pushed(this.divShort);
    $("btn" + this.shortId + "s").src = "/ise/img/rfid_hold_80.png";
    var t = this;
    new PeriodicalExecuter(function(pe) {
      ControlBtn.off(t.divShort);
      $("btn" + t.shortId + "s").src = "/ise/img/rfid_80.png";
      pe.stop();
    }, 1);
  },
  
  onClickLong: function() {
    setDpState(this.longId, 1);
    ControlBtn.pushed(this.divLong);
    $("btn" + this.longId + "l").src = "/ise/img/rfid_hold_80.png";
    var t = this;
    new PeriodicalExecuter(function(pe) {
      ControlBtn.off(t.divLong);
      $("btn" + t.longId + "l").src = "/ise/img/rfid_80.png";
      pe.stop();
    }, 1);
  }
};

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
      translateKey("iconHbThermometer"),
      translateKey("iconHbBattery"),
      translateKey("iconHbRadiatorHorizontal"),
      translateKey("iconHbRadiatorVertical"),
      translateKey("iconHbBathtub"),
      translateKey("iconHbValve"),
      translateKey("iconHbSprinkler")

    ];

    options += "<option name='option_NotUsed' value='-1'>" + translateKey("stringTableNotUsed") + "</option>";
    for (var loop = 0; loop < 27; loop++) {
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
