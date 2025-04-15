DeviceRangeSlider={channels:[],minRange:250,sliderColor:"cornflowerblue",rangeColor:"grey",setup(){this.channels=getChannelMap();var e=document.getElementById("deviceRangesTable");deleteAllChildren(e),(k=document.createElement("div")).classList.add("range_container");var n=document.createElement("div");n.classList.add("form_control");var t=document.createElement("div");t.classList.add("form_control_container_header");var a=document.createElement("label");a.setAttribute("for","allEnabledCheckbox"),a.innerText="All toggle",a.classList.add("range_container_header");var i=document.createElement("input");i.type="checkbox",i.id="allEnabledCheckbox",i.onclick=function(e){let n=document.getElementsByName("ChannelRangeEnabled");for(var t=0;t<n.length;t++)n[t].checked=e.checked,this.toggleChannelEnabled(this.channels[t].name,e.checked);this.updateSettings(0)}.bind(this,i),t.appendChild(i),t.appendChild(a),n.appendChild(t),k.appendChild(n),e.appendChild(k);for(var l=0;l<this.channels.length;l++){var d=this.channels[l];if(isSR6()||!d.sr6Only){var r=d.name,s=d.friendlyName;if(!d)return;var o,c=getTCodeMax();c=d.userMax,o=d.userMin;var h=document.createElement("div");h.classList.add("range_container_header");var m=document.createElement("span"),u=document.createElement("input");u.id=r+"ChannelRangeEnabled",u.setAttribute("name","ChannelRangeEnabled"),u.type="checkbox",u.checked=d.rangeLimitEnabled,u.onclick=function(e,n){e.rangeLimitEnabled=n.checked,this.toggleChannelEnabled(e.name,n.checked),this.updateAllToggleChk(),this.updateSettings()}.bind(this,d,u);var g=document.createElement("label");g.innerText=s,g.setAttribute("for",u.id),m.appendChild(u),m.appendChild(g),h.appendChild(m);var p=document.createElement("div");p.classList.add("form_control_container_header"),p.appendChild(h);var v=document.createElement("div");v.classList.add("sliders_control");var C=document.createElement("input");C.id="minSlider"+r,C.classList.add("fromSlider"),C.type="range",C.min=0,C.max=getTCodeMax()-1,C.value=o,v.appendChild(C);var f=document.createElement("input");f.id="maxSlider"+r,f.type="range",f.min=1,f.max=getTCodeMax(),f.value=c,v.appendChild(f);var b=document.createElement("div");b.classList.add("form_control");var E=document.createElement("div");E.classList.add("form_control_container");var x=document.createElement("label");x.classList.add("form_control_container_label"),x.innerText="Min",x.setAttribute("for","minInput"+r),E.appendChild(x);var T=document.createElement("input");T.id="minInput"+r,T.type="number",T.min=0,T.max=getTCodeMax()-1,T.value=o,E.appendChild(T);var M=document.createElement("div");M.classList.add("form_control_container"),b.appendChild(M);var S=document.createElement("label");S.classList.add("form_control_container_label"),S.innerText="Max",S.setAttribute("for","maxInput"+r),M.appendChild(S);var k,_=document.createElement("input");_.id="maxInput"+r,_.type="number",_.min=1,_.max=getTCodeMax(),_.value=c,M.appendChild(_),b.appendChild(p),b.appendChild(E),b.appendChild(M),(k=document.createElement("div")).classList.add("range_container"),k.appendChild(b),k.appendChild(v),e.appendChild(k),this.fillSlider(C,f,this.rangeColor,this.sliderColor,f),this.setToggleAccessible(f,f),C.oninput=function(e,n,t,a,i){this.controlMinSlider(e,n,t,a,n,i)}.bind(this,C,f,T,_,r),f.oninput=function(e,n,t,a,i){this.controlMaxSlider(e,n,t,a,n,i)}.bind(this,C,f,T,_,r),T.oninput=function(e,n,t,a,i){this.controlMinInput(e,n,t,a,n,i)}.bind(this,C,f,T,_,r),_.oninput=function(e,n,t,a,i){this.controlMaxInput(e,n,t,a,n,i)}.bind(this,C,f,T,_,r)}}this.updateAllToggleChk()},updateSettings(e){updateUserSettings(e,EndPointType.ChannelProfiles.uri,channelsProfileSettings)},updateAllToggleChk(){let e=!1,n=!1;for(var t=0;t<this.channels.length;t++)this.channels[t].rangeLimitEnabled?e=!0:n=!0;let a=document.getElementById("allEnabledCheckbox");n&&e?a.indeterminate=!0:n&&!e?(a.checked=!1,a.indeterminate=!1):!n&&e&&(a.checked=!0,a.indeterminate=!1)},show(){document.getElementById("deviceRangesModal").show()},controlMinInput(e,n,t,a,i,l){const[d,r]=this.getParsed(t,a);var s=d+this.minRange;s>getTCodeMax()&&(s=getTCodeMax()-this.minRange),a.min=s,t.checkValidity()&&a.checkValidity()&&(e.value=t.value,this.setChannelRange(l,parseInt(t.value),r),this.fillSlider(t,a,this.rangeColor,this.sliderColor,i),sendTCode(l+e.value+"S1000"))},controlMaxInput(e,n,t,a,i,l){const[d,r]=this.getParsed(t,a);this.setToggleAccessible(a,n);var s=r-this.minRange;s<getTCodeMin()&&(s=getTCodeMin()+this.minRange),t.max=s,a.checkValidity()&&t.checkValidity()&&(n.value=a.value,this.setChannelRange(l,d,parseInt(a.value)),this.fillSlider(t,a,this.rangeColor,this.sliderColor,i),sendTCode(l+a.value+"S1000"))},controlMinSlider(e,n,t,a,i,l){const[d,r]=this.getParsed(e,n);if(this.fillSlider(e,n,this.rangeColor,this.sliderColor,i),t.value=d,d>=r){var s=d+this.minRange;s>getTCodeMax()&&(s=getTCodeMax(),t.value=s-this.minRange,e.value=s-this.minRange),n.value=s,a.value=s}this.setChannelRange(l,parseInt(t.value),r),sendTCode(l+t.value+"S1000")},controlMaxSlider(e,n,t,a,i,l){const[d,r]=this.getParsed(e,n);if(this.fillSlider(e,n,this.rangeColor,this.sliderColor,i),this.setToggleAccessible(n,n),a.value=r,r<=d){var s=r-this.minRange;s<getTCodeMin()&&(s=getTCodeMin(),a.value=s+this.minRange,n.value=s+this.minRange),e.value=s,t.value=s}this.setChannelRange(l,d,parseInt(a.value)),sendTCode(l+a.value+"S1000")},toggleChannelEnabled(e,n){for(var t=0;t<channelsProfileSettings.channelProfile.length;t++){var a=channelsProfileSettings.channelProfile[t];if(a.name==e){a.rangeLimitEnabled=n;break}}},setChannelRange(e,n,t){for(var a=0;a<channelsProfileSettings.channelProfile.length;a++){var i=channelsProfileSettings.channelProfile[a];if(i.name==e){i.userMin=n,i.userMax=t,this.updateSettings();break}}},getParsed:(e,n)=>[parseInt(e.value,10),parseInt(n.value,10)],fillSlider(e,n,t,a,i){const l=n.max-e.min,d=e.value,r=n.value,s=`linear-gradient(\n        to right,\n        ${t} 0%,\n        ${t} ${d/l*100}%,\n        ${a} ${d/l*100}%,\n        ${a} ${r/l*100}%, \n        ${t} ${r/l*100}%, \n        ${t} 100%)`.replace(/[\n\r\t]/gm,"");i.style.background=s},setToggleAccessible(e,n){Number(e.value)<=0?n.style.zIndex=2:n.style.zIndex=0}};