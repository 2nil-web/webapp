if (typeof app_title === "function") {
    app_title(document.title);
    app_icon("app.ico");
    app_size(650, 220, 1);

    function exit_on_esc() {
        if (event.keyCode === 27) app_exit();
    }
}