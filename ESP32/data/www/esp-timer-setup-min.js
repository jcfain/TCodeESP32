ESPTimer={initialized:!1,model:{},debounces:[],show(){this.modal.show()},setup(){if(this.initialized)return;this.initialized=!0,this.modal=document.getElementById("espTimerSetupModal");let table=Utils.createModalTableSection(this.modal,"Timer setup"),availableTimers=systemInfo.availableTimers;for(let index=0;index<availableTimers.length;index++){const timerObj=availableTimers[index];let timerFrequencyRow=Utils.createNumericFormRow(0,timerObj.name+" (hz)","timerFrequency"+index,pinoutSettings[timerObj.id],50,8e7);timerFrequencyRow.title="Set the frequency of this timer",timerFrequencyRow.input.oninput=function(timerObj,timerFrequencyRow){this.debounces[timerObj.id]&&clearTimeout(this.debounces[timerObj.id]),this.debounces[timerObj.id]=setTimeout((function(){validateIntControl(timerFrequencyRow.input,pinoutSettings,timerObj.id)&&(setRestartRequired(),postPinoutSettings(0))}),defaultDebounce)}.bind(this,timerObj,timerFrequencyRow),table.body.appendChild(timerFrequencyRow.row)}}};