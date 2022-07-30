/* ----------------------------------------------------------------------------
OWLOS DIY Open Source OS for building IoT ecosystems
Copyright 2019, 2020 by:
- Konstantin Brul (konstabrul@gmail.com)
- Vitalii Glushchenko (cehoweek@gmail.com)
- Denys Melnychuk (meldenvar@gmail.com)
- Denis Kirin (deniskirinacs@gmail.com)

This file is part of OWLOS DIY Open Source OS for building IoT ecosystems

OWLOS is free software : you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

OWLOS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with OWLOS. If not, see < https://www.gnu.org/licenses/>.

GitHub: https://github.com/KirinDenis/owlos

(Этот файл — часть OWLOS DIY Open Source OS for building IoT ecosystems.

OWLOS - свободная программа: вы можете перераспространять ее и/или изменять
ее на условиях Стандартной общественной лицензии GNU в том виде, в каком она
была опубликована Фондом свободного программного обеспечения; версии 3
лицензии, любой более поздней версии.

OWLOS распространяется в надежде, что она будет полезной, но БЕЗО ВСЯКИХ
ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА или ПРИГОДНОСТИ ДЛЯ
ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
Подробнее см.в Стандартной общественной лицензии GNU.

Вы должны были получить копию Стандартной общественной лицензии GNU вместе с
этой программой. Если это не так, см. <https://www.gnu.org/licenses/>.)
--------------------------------------------------------------------------------------*/

var F16Widget =

    function (_BaseWidget) {
        "use strict";

        _inheritsLoose(F16Widget, _BaseWidget);

        function F16Widget(parentPanel, id, size) {
            return _BaseWidget.call(this, parentPanel, id, size) || this;
        }

        F16Widget.prototype.onWidgetHolderLoad = function onWidgetHolderLoad(event) {
            _BaseWidget.prototype.onWidgetHolderLoad.call(this, event);
            var rPanel = event.currentTarget;
            var widget = rPanel.widget;

            widget.widgetHolder.className = "col-6 col-sm-4 col-lg-2 widgetHolder";

            //animate
            widget.animated = true;
            widget.levelRectWidth = widget.size / 15;
            widget.levelRectHeight = widget.size / 100;
            widget.levelLeft = widget.width - widget.levelRectWidth + widget.halfPanding;
            widget.levelTop = (widget.height - widget.levelRectHeight * 60 / 2) / 3;
            widget.level1 = [];
            widget.level2 = [];



            widget.levelArc = [];

            for (var i = 1; i < 5; i++) {
                var SVGlevelArc = new SVGArc(widget.SVGViewBox, widget.id + "arcback1" + i, widget.centreX, widget.levelTop, i * widget.radius, widget.size / 34);
                SVGlevelArc.index = i;
                SVGlevelArc.fill = theme.info;

                widget.levelArc.push(SVGlevelArc);
            }


            widget.radar1 = [];
            widget.radar2 = [];
            widget.radar3 = [];
            widget.radar4 = [];

            for (var i = 1; i < 5; i++) {
                var SVGRadarArc1 = new SVGArc(widget.SVGViewBox, widget.id + "arcback1" + i, widget.centreX, widget.topMargin, i * widget.radius, widget.size / 104);
                var SVGRadarArc2 = new SVGArc(widget.SVGViewBox, widget.id + "arcback2" + i, widget.centreX, widget.topMargin, i * widget.radius, widget.size / 94);
                var SVGRadarArc3 = new SVGArc(widget.SVGViewBox, widget.id + "arcback3" + i, widget.centreX, widget.topMargin, i * widget.radius, widget.size / 84);
                var SVGRadarArc4 = new SVGArc(widget.SVGViewBox, widget.id + "arcback4" + i, widget.centreX, widget.topMargin, i * widget.radius, widget.size / 74);
                SVGRadarArc1.index = SVGRadarArc2.index = SVGRadarArc3.index = SVGRadarArc4.index = i;
                SVGRadarArc1.color = SVGRadarArc2.color = SVGRadarArc3.color = SVGRadarArc4.color = theme.warning;
                widget.radar1.push(SVGRadarArc1);
                widget.radar2.push(SVGRadarArc2);
                widget.radar3.push(SVGRadarArc3);
                widget.radar4.push(SVGRadarArc4);
            }

            requestAnimationFrame(function () {
                return widget.animate();
            });

            //Light power slider

            widget.SVGPlusSlider = new SVGIcon(widget.SVGViewBox, plusIcon, 10, 25, 20, 20);
            widget.SVGMinusSlider = new SVGIcon(widget.SVGViewBox, minusIcon, 10, 80, 20, 20);


            //--- animate
            widget._properties.linewidth =
            {
                tab: "G",
                value: 10,
                type: "i"
            };

            widget._properties.rangetype =
            {
                tab: "G",
                value: 'true',
                type: "b"
            };

            widget._properties.min =
            {
                tab: "G",
                value: 0,
                type: "f"
            };

            widget._properties.max =
            {
                tab: "G",
                value: 100,
                type: "f"
            };

            widget._properties.percentbackgroundcolor =
            {
                tab: "C",
                value: theme.secondary,
                type: "c"
            };

            widget._properties.percentbackgroundopacity =
            {
                tab: "O",
                value: 0.5,
                type: "f"
            };

            widget._properties.percentcolor =
            {
                tab: "C",
                value: theme.success,
                type: "c"
            };
            
            widget.clickableToTop();
            widget.proprties = widget._properties;
            widget.doOnLoad();

            widget.SVGViewBox.setAttributeNS(null, "width", widget.size * 2);
            widget.SVGViewBox.setAttributeNS(null, "height", widget.size * 2);

            widget._resize(widget.size);
        }

        F16Widget.prototype._resize = function _resize(size) {
            if (this.size != size) {
                this.SVGViewBox.setAttributeNS(null, "width", size );
                this.SVGViewBox.setAttributeNS(null, "height", size);

            }
            this.size != size

            this.width = this.size;
            this.height = this.size;
            this.centreX = this.width / 2; //  this.centreY = this.height / 2;
            this.widgetTextSize = this.size / 210;
            this.graphWidth = this.width - this.panding;
            this.graphHeight = this.height - this.size / 3.4;
            this.graphTop = this.size / 3.7;

            this.radius = this.size / 5;
            this._properties.linewidth = 4;

            this.topMargin = this.centreY + this.size / 10;
            this.SVGArcSpinner.y = this.topMargin * 2;

            if (this.SVGViewBox == undefined) return;


            this.SVGBackgroundPanel.drawRoundedRect(this.width - 5, this.height - 6, 5, 10, true, true, true, true);
            this.SVGBackdownpanel.drawRoundedRect(this.width - 5, 10, 5, 0, false, false, true, true);
            this.SVGHeaderPanel.drawRoundedRect(this.width, 26, 5, 0, true, true, false, false);

            this.SVGArcBack = new SVGArc(this.SVGViewBox, this.id + "arcback", this.centreX, this.topMargin, this.radius, this._properties.linewidth);
            this.SVGArcWidget = new SVGArc(this.SVGViewBox, this.id + "arcwidget", this.centreX, this.topMargin, this.radius, this._properties.linewidth);

            this.SVGWidgetText.size = this.size / 160;
            this.SVGWidgetText.text = "Auto";
            this.SVGHeaderText.size = this.size / 260;
            

            if (this.SVGLightPowerSlider) {
             //   this.SVGLightPowerSlider.drawRoundedRect(this.width / 20, this.height / 3, 20, 20, true, true, false, false);
                
            }

        }

        F16Widget.prototype.resize = function resize(size) {

            this._resize(size);

        };

        F16Widget.prototype.animate = function animate() {
            var baseWidget2 = this;

            if (this.animated) {
                //animate motion
                for (var i = 0; i < 4; i++) {
                    this.levelArc[i].radius += 1.4;
                    this.levelArc[i].opacity -= 0.01;

                    if (this.levelArc[i].radius > this.radius * 10) {
                        this.levelArc[i].radius = this.radius;
                        this.levelArc[i].opacity = 0.6;
                    }

                    this.levelArc[i].draw(90 + 60, 270 - 60);
                
                //animate light intensive 
                
                    this.radar1[i].radius += 0.5;
                    this.radar1[i].opacity -= 0.01;

                    if (this.radar1[i].radius > this.radius * 2) {
                        this.radar1[i].radius = this.radius / 2;
                        this.radar1[i].opacity = 0.7;
                    }

                    this.radar1[i].draw(270 + 15, 350 - 15);
                    this.radar2[i].radius = this.radar3[i].radius = this.radar4[i].radius = this.radar1[i].radius;
                    this.radar2[i].opacity = this.radar3[i].opacity = this.radar4[i].opacity = this.radar1[i].opacity;
                    this.radar2[i].draw(15, 90 - 15);
                    this.radar3[i].draw(90 + 16, 180 - 15);
                    this.radar4[i].draw(180 + 15, 270 - 15);
                }


                requestAnimationFrame(function () {
                    return baseWidget2.animate();
                });
            }
        };


        F16Widget.prototype.drawWidget = function drawWidget() {
            _BaseWidget.prototype.drawWidget.call(this);

            if (this.SVGArcBack == undefined) return;

            var _data = this.data;
            if (this._properties.rangetype.value !== 'true') { //когда randge
                var range = this._properties.max.value - this._properties.min.value;
                _data = this.data / (range / 100);
            }
            else {
                
                if ((_data > 100) || (_data < 0)) {
                    this.toColor(this.SVGArcWidget, theme.warning);
                }
            }

            var oneHangPercent = 360 + 90 + 30 - 240;
            var drawPercent = _data * (oneHangPercent / 100); //back f16 widget

            this.SVGArcBack.linewidth = this._properties.linewidth.value;
            this.SVGArcWidget.linewidth = this._properties.linewidth.value;

            this.SVGArcBack.color = this._properties.percentbackgroundcolor.value;
            this.SVGArcBack.opacity = this._properties.percentbackgroundopacity.value;

            this.SVGArcBack.draw(240, 240 + oneHangPercent); //f16 widget

            this.SVGArcWidget.draw(240, 240 + drawPercent);

            this.SVGPlusSlider.opacity = 1.0;
            this.SVGPlusSlider.fill = theme.info;

            this.SVGMinusSlider.opacity = 1.0;
            this.SVGMinusSlider.fill = theme.info;



            switch (this._networkStatus) {
                case NET_ONLINE:
                    this.toColor(this.SVGArcWidget, theme.warning);
                    break;

                case NET_ERROR:
                    this.toColor(this.SVGArcWidget, theme.warning);
                    break;

                case NET_RECONNECT:
                    this.toColor(this.SVGArcWidget, theme.warning);
                    break;

                default:
                    //offline
                    this.toColor(this.SVGArcWidget, theme.warning);
                    break;
            }
        };

        return F16Widget;
    }(BaseWidget);