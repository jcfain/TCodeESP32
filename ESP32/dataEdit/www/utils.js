/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */


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
    /** If isVisible is undefined the element will be toggled */
    toggleElementShown(element, isVisible, animate) {
        //element.hidden = !isVisible;// Broken with display:flex
        let className = "hidden";
        if(animate) {
            className = "hidden-transition";
            if(!element.classList.contains("max-height-transition")) {
                element.classList.add("max-height-transition")
            }
        }
        if(isVisible == undefined ) {
            element.classList.toggle(className);
            return;
        }
        if(!isVisible)
            element.classList.add(className);
        else
            element.classList.remove(className);
    },
    toggleControlVisibilityByID(id, isVisible, animate, elementCallback) {
        var control = document.getElementById(id);
            this.toggleElementShown(control, isVisible, animate);
            if(elementCallback)
                elementCallback(control);
    },
    toggleControlVisibilityByName(name, isVisible, animate, elementCallback) {
        var controls = document.getElementsByName(name);
        for(var i=0;i < controls.length; i++) {
            this.toggleElementShown(controls[i], isVisible, animate);
            if(elementCallback)
                elementCallback(controls[i]);
        }
    },
    toggleControlVisibilityByClassName(className, isVisible, animate, elementCallback) {
        var controls = document.getElementsByClassName(className);
        for(var i=0;i < controls.length; i++) {
            this.toggleElementShown(controls[i], isVisible, animate);
            if(elementCallback)
                elementCallback(controls[i]);
        }
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
    createFormButtonRow(rowID, buttonID, buttonLabel) {
        const row = this.createFormRow(rowID);
        const emptyCell = this.createFormCell();
        const buttonCell = this.createFormCell();
        const button = document.createElement("button");
        button.innerText = buttonLabel;
        button.id = buttonID;
        buttonCell.appendChild(button);
        row.appendChild(emptyCell);
        row.appendChild(buttonCell);
        return {row: row, emptyCell: emptyCell, buttonCell: buttonCell, button: button};
    },
    createTextInput(id, value, maxLength, callback) {
        const input = document.createElement("input");
        input.type = "text";
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
    createTextAreaInput(id, value, maxLength, callback) {
        const input = document.createElement("textarea");
        input.type = "text";
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
    createTextAreaFormRow(rowID, name, textInputID, value, maxLength, callback) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell(0, name);
        const valueCell = this.createFormCell();
        const input = this.createTextAreaInput(textInputID, value, maxLength, callback);
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
    },
    createLabelFormRow(rowID, name) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell(0, name);
        const valueCell = this.createFormCell();
        row.appendChild(nameCell);
        row.appendChild(valueCell);
        return {row: row, input: undefined, nameCell: nameCell, valueCell: valueCell};
    },
    createInstructionRow(rowID, instruction) {
        const row = this.createFormRow(rowID);
        const nameCell = this.createFormCell();
        const valueCell = this.createFormCell();
        const span = document.createElement("span");
        span.classList.add("instruction-text")
        span.innerHTML = instruction
        valueCell.appendChild(span);
        row.appendChild(nameCell);
        row.appendChild(valueCell);
        return {row: row, input: undefined, nameCell: nameCell, valueCell: valueCell};
    },
    createTableSection() {
        var rootdiv = document.createElement("div");
        rootdiv.classList.add("formTable");
        rootdiv.style = "box-shadow: none; width: 100%;"
        var parent = document.createElement("div");
        var header = document.createElement("div");
        header.classList.add("tHeader")
        rootdiv.appendChild(header);
        rootdiv.appendChild(parent);

        var tableDiv = document.createElement("div");
        tableDiv.classList.add("formTable");
        tableDiv.style = "box-shadow: none; width: auto; margin: 0; padding: 0;"
        var tableBody = document.createElement("div");
        tableDiv.appendChild(tableBody);
        parent.appendChild(tableDiv);
        return {root: rootdiv, table: tableDiv, body: tableBody, header: header, parent: parent}
    },
    createModalTableSection(modalElement, modelTitle) {
        let title = document.createElement("span");
        title.setAttribute("slot", "title");
        title.innerText = modelTitle
        let table = this.createTableSection();
        modalElement.appendChild(title);
        modalElement.appendChild(table.header);
        modalElement.appendChild(table.root);
        return table;
    }
}
