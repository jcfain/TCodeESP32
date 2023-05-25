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
        Utils.toggleElementShown(control, isVisible);
    },
    toggleControlVisibilityByName(name, isVisible) {
        var controls = document.getElementsByName(name);
        for(var i=0;i < controls.length; i++)
            Utils.toggleElementShown(controls[i], isVisible);
    },
    toggleControlVisibilityByClassName(className, isVisible) {
        var controls = document.getElementsByClassName(className);
        for(var i=0;i < controls.length; i++)
            Utils.toggleElementShown(controls[i], isVisible);
    }
}
