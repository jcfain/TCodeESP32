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

Buttons = {
    templates: [],
    setNameDebounce: undefined,
    updateDebounce: undefined,
    setup() {
        document.getElementById("bootButtonEnabled").checked = buttonSettings["bootButtonEnabled"];
        document.getElementById("buttonSetsEnabled").checked = buttonSettings["buttonSetsEnabled"];
        document.getElementById("bootButtonCommand").value = buttonSettings["bootButtonCommand"].replace(" ", "\n");
        document.getElementById("buttonAnalogDebounce").value = buttonSettings["buttonAnalogDebounce"];
        removeByClass("buttonSetRow");
        buttonSettings["buttonSets"].forEach((buttonSet, setIndex) => {

            const buttons = buttonSet["buttons"];

            //Main UI template
            const buttonSetsDiv = document.getElementById("buttonControls");

            let buttonSetNameRow = Utils.createTextFormRow(0, "Name", 'buttonSetName'+setIndex, buttonSet.name, 25, function(setIndex) {this.update(setIndex)}.bind(this, setIndex));
            buttonSetNameRow.input.setAttribute("readonly", true);
            let buttonSetPinRow = Utils.createNumericFormRow(0, "Pin", 'buttonSetPin'+setIndex, buttonSet.pin, -1, 255, function(setIndex) {this.update(setIndex)}.bind(this, setIndex));
            buttonSetPinRow.input.name = "buttonSetPins"
            buttonSetPinRow.input.setAttribute("readonly", true);
            buttonSetPinRow.title = `The pin this button set is on`

            var buttonSetEditButton = document.createElement("div");
            buttonSetEditButton.classList.add("tRow");
            //buttonSetEditButton.classList.add("motion-profile-edit-channel-header");
            var cell1 = document.createElement("div");
            cell1.classList.add("tCell");
            var cell2 = document.createElement("div");
            cell2.classList.add("tCell");
            var label = document.createElement("span");
            label.innerText = "";//buttonSet["name"];
            var button = document.createElement("button");
            button.id = "ButtonSetEditButton" + setIndex;
            button.innerText = "Edit";
            button.onclick = function(setIndex) {
                this.editButtonSet(setIndex);
            }.bind(this, setIndex);
            cell1.appendChild(label);
            cell2.appendChild(button);
            buttonSetEditButton.appendChild(cell1);
            buttonSetEditButton.appendChild(cell2);

            buttonSetNameRow.row.classList.add("buttonSetRow");
            buttonSetPinRow.row.classList.add("buttonSetRow");
            buttonSetEditButton.classList.add("buttonSetRow");

            buttonSetsDiv.appendChild(buttonSetNameRow.row);
            buttonSetsDiv.appendChild(buttonSetPinRow.row);
            buttonSetsDiv.appendChild(buttonSetEditButton);


            //Modal Template
            var modalRootdiv = document.createElement("div");
            modalRootdiv.classList.add("formTable");
            modalRootdiv.style = "box-shadow: none; width: 100%;"
            modalRootdiv.id = "buttonContainer"+setIndex;
            modalRootdiv.name = "buttonContainer";
            var modalParent = document.createElement("div");
            var modalheader = document.createElement("div");
            modalheader.classList.add("tHeader")
            modalParent.appendChild(modalheader);

            const buttonTable = document.createElement("div");
            buttonTable.id = "buttonTable" + setIndex;
            buttonTable.setAttribute("name", "buttonTable");
            buttonTable.classList.add("formTable");
            buttonTable.style = "box-shadow: none; width: auto; margin: 0; padding: 0;"
            const buttonTableDiv = document.createElement("div");

            let buttonSetNameRowEdit = Utils.createTextFormRow(0, "Name", 'buttonSetNameEdit'+setIndex, buttonSet.name, 25, function(setIndex) {
                this.update(setIndex);
            }.bind(this, setIndex));
            buttonSetNameRowEdit.input.required = true;
            let buttonSetPinRowEdit  = Utils.createNumericFormRow(0, "Pin", 'buttonSetPinEdit'+setIndex, buttonSet.pin, -1, 255, function(setIndex) {
                this.update(setIndex);
            }.bind(this, setIndex));
            buttonSetPinRowEdit.input.required = true;
            buttonSetPinRowEdit.title = `The pin this button set is on`

            buttonTableDiv.appendChild(buttonSetNameRowEdit.row);
            buttonTableDiv.appendChild(buttonSetPinRowEdit.row);

            const buttonsRow = Utils.createLabelFormRow(0, "Set Buttons");
            buttonTableDiv.appendChild(buttonsRow.row);

            for(var i = 0; i < buttons.length; i++) {
                const buttonIndex = i;


                let buttonRow = Utils.createTextFormRow(0, "Name", 'buttonName'+setIndex+buttonIndex, buttons[i].name, 25, function(setIndex, buttonIndex) {
                    if(validateStringControl("buttonName"+setIndex+buttonIndex, buttonSettings['buttonSets'][setIndex]["buttons"][buttonIndex], "name")) {
                        this.update(setIndex)
                    }
                }.bind(this, setIndex, buttonIndex));
                buttonRow.title = `Name of the button`
                buttonRow.input.required = true;
                buttonTableDiv.appendChild(buttonRow.row);

                buttonRow = Utils.createTextAreaFormRow(0, "Command", 'buttonCommand'+setIndex+buttonIndex, buttons[i].command, 255, function(setIndex, buttonIndex) {
                    if(validateStringControl("buttonCommand"+setIndex+buttonIndex, buttonSettings['buttonSets'][setIndex]["buttons"][buttonIndex], "command")) {
                        this.update(setIndex)
                    }
                }.bind(this, setIndex, buttonIndex));
                buttonRow.title = `This is the TCode command executed when the button is pressed`
                buttonTableDiv.appendChild(buttonRow.row);

                // buttonRow = Utils.createNumericFormRow(0, "Index", 'buttonIndex'+setIndex+buttonIndex, buttons[i].index, 0, buttons.length - 1, this.update);
                // buttonRow.title = `This is the index the button is physically on in the resistor ladder.`
                // buttonTableDiv.appendChild(buttonRow.row);

            };
            let span = document.createElement("span");
            span.classList.add("instruction-text")
            span.innerHTML = `
            Instructions:<br>
            You can enter any combination of TCode commands separated by spaces in the inputs above.
            For example: '#motion-disable #resume #device-home #ok'
            NOTE: There is currently no way to delay between the command execution.
            `

            modalParent.appendChild(buttonTable);
            buttonTable.appendChild(buttonTableDiv);
            modalParent.appendChild(span);
            modalRootdiv.appendChild(modalParent);
            this.templates.push(modalRootdiv);

        });
    },
    update(setIndex, debounce) {
        if(this.updateDebounce) {
            clearTimeout(this.updateDebounce);
        }
        this.updateDebounce = setTimeout(function(setIndex) {
            let valid = true;
            const enabled = document.getElementById('bootButtonEnabled').checked;
            if(buttonSettings["bootButtonEnabled"] != enabled) {
                buttonSettings["bootButtonEnabled"] = enabled;
                showRestartRequired();
            }
            const setsEnabled = document.getElementById('buttonSetsEnabled').checked;
            if(buttonSettings["buttonSetsEnabled"] != setsEnabled) {
                buttonSettings["buttonSetsEnabled"] = setsEnabled;
                showRestartRequired();
            }

            buttonSettings["bootButtonCommand"] = document.getElementById('bootButtonCommand').value.replace("\n", " ");
            if(validateIntControl("buttonAnalogDebounce", buttonSettings, "buttonAnalogDebounce")) {
                buttonSettings["buttonAnalogDebounce"] = parseInt(document.getElementById("buttonAnalogDebounce").value);
            } else {
                valid = false;
            }
            if(setIndex != undefined) {
                if(validateIntControl("buttonSetNameEdit"+setIndex, buttonSettings['buttonSets'][setIndex]["name"], "name")) {
                    document.getElementById('buttonSetName'+setIndex).value = document.getElementById('buttonSetNameEdit'+setIndex).value;
                } else {
                    valid = false;
                }
                if(validateStringControl("buttonSetPinEdit"+setIndex, buttonSettings['buttonSets'][setIndex]["pin"], "pin")) {
                    document.getElementById('buttonSetPin'+setIndex).value = document.getElementById('buttonSetPinEdit'+setIndex).value;
                } else {
                    valid = false;
                }
                if(validateStringControl("buttonSetName"+setIndex, buttonSettings['buttonSets'][setIndex], "name")) {
                    buttonSettings["buttonSets"][setIndex]["name"] = document.getElementById('buttonSetNameEdit'+setIndex).value;
                } else {
                    valid = false;
                }
                if(validatePins()) {
                    this.updatebuttonSet(setIndex);
                } else {
                    valid = false;
                }
            } else {
                valid = validatePins();
            }
            if(valid)
                postButtonSettings(0);
        }.bind(this, setIndex), debounce ? debounce : 3000);
        // if(setIndex != undefined) {
        //     this.updatebuttonSet(setIndex);
        // }
        // updateUserSettings(0, EndPointType.Buttons.uri, buttonSettings);
    },
    editButtonSet(index) {
        const modal = document.getElementById("buttonSetsModal");
        removeAllChildren(modal);
        modal.appendChild(this.templates[index]);
        this.setbuttonSet(index);
        const header = document.createElement("span");
        header.innerText = "Edit button set"
        header.setAttribute("slot", "title");
        modal.appendChild(header);
        modal.show();
    },
    setbuttonSet(setIndex) {
        const buttons = buttonSettings["buttonSets"][setIndex]["buttons"];
        for(var i = 0; i < buttons.length; i++) {
            const buttonIndex = i;
            const button = buttons[i];
            document.getElementById('buttonName'+setIndex+buttonIndex).value = button.name;
            document.getElementById('buttonCommand'+setIndex+buttonIndex).value = button.command;
            // document.getElementById('buttonIndex'+setIndex+buttonIndex).value = button[i].index;
        };
    },
    updatebuttonSet(setIndex) {
        buttonSettings["buttonSets"][setIndex]["name"] = document.getElementById('buttonSetNameEdit'+setIndex).value;
        buttonSettings["buttonSets"][setIndex]["pin"] = parseInt(document.getElementById('buttonSetPinEdit'+setIndex).value);
        const buttons = buttonSettings["buttonSets"][setIndex]["buttons"];
        for(var i = 0; i < buttons.length; i++) {
            const buttonIndex = i;
            //const button = buttons[i];
            const nameNode = document.getElementById('buttonName'+setIndex+buttonIndex);
            if(nameNode) {
                buttonSettings["buttonSets"][setIndex]["buttons"][i]["name"] = document.getElementById('buttonName'+setIndex+buttonIndex).value;
                buttonSettings["buttonSets"][setIndex]["buttons"][i]["command"] = document.getElementById('buttonCommand'+setIndex+buttonIndex).value.replace("\n", " ");
            }
            // document.getElementById('buttonIndex'+setIndex+buttonIndex).value = button[i].index;
        };
    },

    setButtonSetName(profileIndex) {
        if(this.setNameDebounce) {
            clearTimeout(this.setNameDebounce);
        }

        this.setNameDebounce = setTimeout(function(profileIndex) {
            if(validateStringControl("buttonSetName"+profileIndex, buttonSettings['buttonSets'][profileIndex], "name")) {
                //updateMotionProfileName(profileIndex);
                postButtonSettings(0);
            }
        }.bind(this, profileIndex), 3000);
    },
    isButtonSetsEnabled() {
        return buttonSettings["buttonSetsEnabled"];
    },
    isBootButtonEnabled() {
        return buttonSettings["bootButtonEnabled"];
    }
}