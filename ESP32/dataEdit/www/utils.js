Utils = {
    round2(value) {
        return Math.round(value * 100) / 100;
    },
    _debounces: [{name: "Debounce1", timeout: 0, delay: 0, paramsArray: undefined}],
    debounce(name, methodToDebounce, delayInMS, ...methodToDebounceParamsArray) {
        if(this._debounces[name]) {
            Object.keys(this._debounces).forEach(x => {
                clearTimeout(this._debounces[x].timeout);
            });
        } else {
            this._debounces[name] = {name: name, timeout: 0, delay: delayInMS, paramsArray: methodToDebounceParamsArray};
        }
        Object.keys(this._debounces).forEach(x => {
            this._debounces[x].timeout = setTimeout(() => {
                if(this._debounces[x].paramsArray)
                    methodToDebounce(...this._debounces[x].paramsArray);
                else
                    methodToDebounce();
                delete this._debounces[x];
            }, this._debounces[x].delay);
        });
    },
    toggleElementShown(element, isVisible) {
        //element.hidden = !isVisible;// Broken with display:flex
        if(!isVisible)
            element.classList.add("hidden");
        else
            element.classList.remove("hidden");
    },
    toggleControlVisibilityByID(id, isVisible) {
        var control = document.getElementById(id);
            this.toggleElementShown(control, isVisible);
    },
    toggleControlVisibilityByName(name, isVisible) {
        var controls = document.getElementsByName(name);
        for(var i=0;i < controls.length; i++)
            this.toggleElementShown(controls[i], isVisible);
    },
    toggleControlVisibilityByClassName(className, isVisible) {
        var controls = document.getElementsByClassName(className);
        for(var i=0;i < controls.length; i++)
            this.toggleElementShown(controls[i], isVisible);
    },
    createFormRow(id) {
        const row = document.createElement("div");
        row.classList.add("tRow");
        if(id) {
            row.id = id;
        }
        return row;
    },
    createFormCell(id, name, inputID, value) {
        const cell = document.createElement("div");
        cell.classList.add("tCell");
        if(id) {
            cell.id = id;
        }
        if(name) {
            const span = document.createElement("span");
            span.innerText = name;
            cell.appendChild(span);
        }
        if(inputID) {
            this.createTextInput(inputID, value, cell);
        }
        return cell;
    },
    createTextInput(id, value, maxLength, callback) {
        const input = document.createElement("input");
        if(id) {
            input.id = id;
        }
        if(value) {
            input.value = value;
        }
        if(maxLength) {
            input.maxLength = maxLength;
        }
        if(callback) {
            input.oninput = callback;
        }
        return input;
    },
    createCheckboxInput(id, checked, callback) {
        const input = document.createElement("input");
        input.type = "checkbox";
        if(id) {
            input.id = id;
        }
        if(checked) {
            input.checked = checked;
        }
        if(callback) {
            input.onclick = callback;
        }
        return input;
    },
    createNumericInput(id, value, min, max, callback) {
        const input = document.createElement("input");
        input.type = "number";
        if(id) {
            input.id = id;
        }
        if(value) {
            input.value = value;
        }
        if(min) {
            input.min = min;
        }
        if(max) {
            input.max = max;
        }
        if(callback) {
            input.oninput = callback;
        }
        return input;
    },
    createTextFormRow(rowID, name, textInputID, value, maxLength, callback) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell(0, name);
        const valueCell = this.createFormCell();
        const input = this.createTextInput(textInputID, value, maxLength, callback);
        valueCell.appendChild(input);
        row.appendChild(nameCell);
        row.appendChild(valueCell);
        return {row: row, input: input, nameCell: nameCell, valueCell: valueCell};
    },
    createNumericFormRow(rowID, name, inputID, value, min, max, callback) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell(0, name);
        const valueCell = this.createFormCell();
        const input = this.createNumericInput(inputID, value, min, max, callback);
        valueCell.appendChild(input);
        row.appendChild(nameCell);
        row.appendChild(valueCell);
        return {row: row, input: input, nameCell: nameCell, valueCell: valueCell};
    },
    createCheckboxFormRow(rowID, name, inputID, checked, callback) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell(0, name);
        const valueCell = this.createFormCell();
        const input = this.createCheckboxInput(inputID, checked, callback);
        valueCell.appendChild(input);
        row.appendChild(nameCell);
        row.appendChild(valueCell);
        return {row: row, input: input, nameCell: nameCell, valueCell: valueCell};
    }
}
