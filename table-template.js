let g_mode = "";
let g_filterText = "";
let g_include = true;
let g_regex = false;
let g_searchColumn = -1;



// Hide or show certain rows of the table
function setTableFilter() {

    // Hide or show rows having at least one given class
    const showByClass = (classes, show) => {
        for (const className of classes) {
            const rows = document.getElementsByClassName(className);

            for (let i = 0; i < rows.length; i++) {
                rows[i].hidden = !show;
            }
        }
    };


    if (g_mode == "all") {
        showByClass(["row"], true);
    }

    if (g_mode == "fail") {
        showByClass(["row"], false);
        showByClass(["row-fail"], true);
    }

    if (g_mode == "timeout") {
        showByClass(["row"], false);
        showByClass(["row-timeout"], true);
    }

    if (g_mode == "bad") {
        showByClass(["row"], false);
        showByClass(["row-fail"], true);
        showByClass(["row-timeout"], true);
    }
}

function setTableFilterText() { 
    const table = document.getElementById("data-table");
    const rows = table.rows;

    let matchFunction;

    if (g_regex) {
        const re = new RegExp(g_filterText);

        matchFunction = (text) => {
            return re.test(text)
        };

    } else {
        matchFunction = (text) => {
            return text.includes(g_filterText);
        };
    }


    for (let i = 1; i < rows.length; i++) {
        const row = rows[i];

        if (row.hidden) {
            continue;
        }

        const divs = row.getElementsByTagName("div");


        let foundMatch = false;
        if (g_searchColumn == -1) {
            for (let i = 0; i < divs.length; i++) {
                const divText = divs[i].innerHTML;

                if (matchFunction(divText)) {
                    foundMatch = true;
                    break;
                }
            }
        } else {
            foundMatch = matchFunction(divs[g_searchColumn].innerHTML);
        }

        if (foundMatch != g_include) {
            row.hidden = true;
        }
    }


}

function refresh() {
    setTableFilter();
    setTableFilterText();
}



// Wait for the document to finish loading before doing anything
document.addEventListener("DOMContentLoaded", () => {
    const dropdown = document.getElementById("outcome-filter-dropdown");
    //const table = document.getElementById("data-table");

    const filterTextInput = document.getElementById("filter-text-input");
    const filterTextExclude = document.getElementById("filter-text-exclude");
    const filterTextRegex = document.getElementById("filter-text-regex");
    const filterColumnSelect = document.getElementById("filter-text-column-select");

    g_mode = dropdown.value;
    g_filterText = filterTextInput.value;
    g_include = !filterTextExclude.checked;
    g_regex = filterTextRegex.checked;
    g_searchColumn = filterColumnSelect.value;


    filterTextInput.addEventListener("input", (e) => {
        g_filterText = e.target.value;
        refresh();
    });

    dropdown.addEventListener("change", (e) => {
        g_mode = e.target.value;
        refresh();
    });

    filterTextExclude.addEventListener("change", (e) => {
        g_include = !e.target.checked;
        refresh();
    });

    filterTextRegex.addEventListener("change", (e) => {
        g_regex = e.target.checked;
        refresh();
    });

    filterColumnSelect.addEventListener("change", (e) => {
        g_searchColumn = e.target.value;
        refresh();
    });


    refresh();

});
