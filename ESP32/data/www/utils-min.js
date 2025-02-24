Utils={round2:e=>Math.round(100*e)/100,_debounces:[{name:"Debounce1",timeout:0,delay:0,paramsArray:void 0}],debounce(e,t,n,...a){this._debounces[e]?Object.keys(this._debounces).forEach((e=>{clearTimeout(this._debounces[e].timeout)})):this._debounces[e]={name:e,timeout:0,delay:n,paramsArray:a},Object.keys(this._debounces).forEach((e=>{this._debounces[e].timeout=setTimeout((()=>{this._debounces[e].paramsArray?t(...this._debounces[e].paramsArray):t(),delete this._debounces[e]}),this._debounces[e].delay)}))},toggleElementShown(e,t,n){let a="hidden";n&&(a="hidden-transition",e.classList.contains("max-height-transition")||e.classList.add("max-height-transition")),null!=t?t?e.classList.remove(a):e.classList.add(a):e.classList.toggle(a)},toggleControlVisibilityByID(e,t,n,a){var r=document.getElementById(e);this.toggleElementShown(r,t,n),a&&a(r)},toggleControlVisibilityByName(e,t,n,a){for(var r=document.getElementsByName(e),l=0;l<r.length;l++)this.toggleElementShown(r[l],t,n),a&&a(r[l])},toggleControlVisibilityByClassName(e,t,n,a){for(var r=document.getElementsByClassName(e),l=0;l<r.length;l++)this.toggleElementShown(r[l],t,n),a&&a(r[l])},createFormRow(e){const t=document.createElement("div");return t.classList.add("tRow"),e&&(t.id=e),t},createFormCell(e,t,n,a){const r=document.createElement("div");if(r.classList.add("tCell"),e&&(r.id=e),t){const e=document.createElement("span");e.innerText=t,r.appendChild(e)}return n&&this.createTextInput(n,a,r),r},createFormButtonRow(e,t,n){const a=this.createFormRow(e),r=this.createFormCell(),l=this.createFormCell(),o=document.createElement("button");return o.innerText=n,o.id=t,l.appendChild(o),a.appendChild(r),a.appendChild(l),{row:a,emptyCell:r,buttonCell:l,button:o}},createTextInput(e,t,n,a){const r=document.createElement("input");return r.type="text",e&&(r.id=e),t&&(r.value=t),n&&(r.maxLength=n),a&&(r.oninput=a),r},createTextAreaInput(e,t,n,a){const r=document.createElement("textarea");return r.type="text",e&&(r.id=e),t&&(r.value=t),n&&(r.maxLength=n),a&&(r.oninput=a),r},createCheckboxInput(e,t,n){const a=document.createElement("input");return a.type="checkbox",e&&(a.id=e),t&&(a.checked=t),n&&(a.onclick=n),a},createNumericInput(e,t,n,a,r){const l=document.createElement("input");return l.type="number",e&&(l.id=e),t&&(l.value=t),n&&(l.min=n),a&&(l.max=a),r&&(l.oninput=r),l},createTextFormRow(e,t,n,a,r,l){const o=this.createFormRow(e),i=this.createFormCell(0,t),d=this.createFormCell(),c=this.createTextInput(n,a,r,l);return d.appendChild(c),o.appendChild(i),o.appendChild(d),{row:o,input:c,nameCell:i,valueCell:d}},createTextAreaFormRow(e,t,n,a,r,l){const o=this.createFormRow(e),i=this.createFormCell(0,t),d=this.createFormCell(),c=this.createTextAreaInput(n,a,r,l);return d.appendChild(c),o.appendChild(i),o.appendChild(d),{row:o,input:c,nameCell:i,valueCell:d}},createNumericFormRow(e,t,n,a,r,l,o){const i=this.createFormRow(e),d=this.createFormCell(0,t),c=this.createFormCell(),s=this.createNumericInput(n,a,r,l,o);return c.appendChild(s),i.appendChild(d),i.appendChild(c),{row:i,input:s,nameCell:d,valueCell:c}},createCheckboxFormRow(e,t,n,a,r){const l=this.createFormRow(e),o=this.createFormCell(0,t),i=this.createFormCell(),d=this.createCheckboxInput(n,a,r);return i.appendChild(d),l.appendChild(o),l.appendChild(i),{row:l,input:d,nameCell:o,valueCell:i}},createLabelFormRow(e,t){const n=this.createFormRow(e),a=this.createFormCell(0,t),r=this.createFormCell();return n.appendChild(a),n.appendChild(r),{row:n,input:void 0,nameCell:a,valueCell:r}},createInstructionRow(e,t){const n=this.createFormRow(e),a=this.createFormCell(),r=this.createFormCell(),l=document.createElement("span");return l.classList.add("instruction-text"),l.innerHTML=t,r.appendChild(l),n.appendChild(a),n.appendChild(r),{row:n,input:void 0,nameCell:a,valueCell:r}},createTableSection(){var e=document.createElement("div");e.classList.add("formTable"),e.style="box-shadow: none; width: 100%;";var t=document.createElement("div"),n=document.createElement("div");n.classList.add("tHeader"),e.appendChild(n),e.appendChild(t);var a=document.createElement("div");a.classList.add("formTable"),a.style="box-shadow: none; width: auto; margin: 0; padding: 0;";var r=document.createElement("div");return a.appendChild(r),t.appendChild(a),{root:e,table:a,body:r,header:n,parent:t}},createModalTableSection(e,t){let n=document.createElement("span");n.setAttribute("slot","title"),n.innerText=t;let a=this.createTableSection();return e.appendChild(n),e.appendChild(a.header),e.appendChild(a.root),a}};