

// Hide or show certain rows of the table
function setTableFilter(table, mode) {

    // Hide or show rows having at least one given class
    const showByClass = (classes, show) => {
        for (const className of classes) {
            const rows = document.getElementsByClassName(className);

            for (let i = 0; i < rows.length; i++) {
                rows[i].hidden = !show;
            }
        }
    };


    if (mode == "all") {
        showByClass(["row-data"], true);
    }

    if (mode == "fail") {
        showByClass(["row-data"], false);
        showByClass(["row-fail"], true);
    }

    if (mode == "timeout") {
        showByClass(["row-data"], false);
        showByClass(["row-timeout"], true);
    }

    if (mode == "bad") {
        showByClass(["row-data"], false);
        showByClass(["row-fail"], true);
        showByClass(["row-timeout"], true);
    }
}


// Wait for the document to finish loading before doing anything
document.addEventListener("DOMContentLoaded", () => {
    const dropdown = document.getElementById("outcome-filter-dropdown");
    const table = document.getElementById("data-table");

    dropdown.addEventListener("change", (e) => {
        const value = e.target.value;
        setTableFilter(table, value);
    });

    setTableFilter(table, dropdown.value);

});
