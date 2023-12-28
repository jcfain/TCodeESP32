Buttons = {
    setup() {
        document.getElementById("bootButtonCommand").value = userSettings["bootButtonCommand"].replace(" ", "\n");
    },
    update() {
        userSettings["bootButtonCommand"] = document.getElementById('bootButtonCommand').value.replace("\n", " ");;
        updateUserSettings();
    }
}