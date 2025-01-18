let g_mode = "";
let g_filterText = "";
let g_include = true;
let g_regex = false;
let g_searchColumn = -1;

function showHideIndices() {
    const indexRow = document.getElementById("col-indices");

    if (g_searchColumn != -2) {
        indexRow.hidden = true;
    } else {
        indexRow.hidden = false;
    }
}

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

    if (g_mode == "problem") {
        showByClass(["row"], false);
        showByClass(["row-fail"], true);
        showByClass(["row-timeout"], true);
        showByClass(["row-bad-hash"], true);
        showByClass(["row-completed-different"], true);
    }

    if (g_mode == "fail") {
        showByClass(["row"], false);
        showByClass(["row-fail"], true);
    }

    if (g_mode == "timeout") {
        showByClass(["row"], false);
        showByClass(["row-timeout"], true);
    }

    if (g_mode == "completed-different") {
        showByClass(["row"], false);
        showByClass(["row-completed-different"], true);
    }

    if (g_mode == "hash") {
        showByClass(["row"], false);
        showByClass(["row-bad-hash"], true);
    }

    if (g_mode == "no-problem") {
        showByClass(["row"], true);
        showByClass(["row-fail"], false);
        showByClass(["row-timeout"], false);
        showByClass(["row-bad-hash"], false);
        showByClass(["row-completed-different"], false);
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


    for (let i = 2; i < rows.length; i++) {
        const row = rows[i];

        if (row.hidden) {
            continue;
        }

        const divs = row.getElementsByTagName("div");


        let foundMatch = false;
        if (g_searchColumn == -1) { // Search all columns
            for (let i = 0; i < divs.length; i++) {
                const divText = divs[i].innerHTML;

                if (matchFunction(divText)) {
                    foundMatch = true;
                    break;
                }
            }
        } else if (g_searchColumn == -2) { // "COMBINE AND TAG"
            let rowText = "";

            for (let i = 0; i < divs.length; i++) {
                rowText += "(COL" + i.toString() + ")" + divs[i].innerHTML;
            }

            if (matchFunction(rowText)) {
                foundMatch = true;
            }
        } else { // Search specific column
            foundMatch = matchFunction(divs[g_searchColumn].innerHTML);
        }

        if (foundMatch != g_include) {
            row.hidden = true;
        }
    }


}

function refresh() {
    showHideIndices();
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

    const problemSummary = document.getElementById("problem-summary");

    let problems = [
        {
            "row-css": "row-fail",
            "new-css": "cell-new-fail",
            "main-text": "test(s) failed",
            "secondary-text": "newly failing",
        },

        {
            "row-css": "row-timeout",
            "new-css": "cell-new-timeout",
            "main-text": "test(s) timed out",
            "secondary-text": "newly timed out",
        },

        {
            "row-css": "row-completed-different",
            "main-text": "\"completed\" test(s) with different values",
        },


        {
            "row-css": "row-bad-hash",
            "main-text": "test(s) with non-matching hashes",
        },
    ];


    let summaryText = "";
    for (const problem of problems) {
        const rows = document.getElementsByClassName(problem["row-css"]);
        let regressionCount = 0;

        if (problem["new-css"]) {
            const newCSS = problem["new-css"];

            for (let i = 0; i < rows.length; i++) {
                let newCells = rows[i].getElementsByClassName(newCSS);
                if (newCells.length > 0) {
                    regressionCount++;
                }
            }
        }

        if (rows.length > 0) {
            summaryText += rows.length.toString() + " " + problem["main-text"];

            if (regressionCount > 0) {
                summaryText += " (" + regressionCount.toString() + " " + problem["secondary-text"] + ")";
            }

            summaryText += "\n";
        }

    }
    if (summaryText.length === 0) {
        summaryText = "No problems found. All tests passed!";
    }
    problemSummary.innerHTML = summaryText;

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


    const indexRow = document.getElementById("col-indices");
    const indexRowCells = indexRow.getElementsByTagName("th"); 
    for (const th of indexRowCells) {
        th.addEventListener("click", (e) => {
            let indexText = th.innerHTML;
            indexText = indexText.replace("(", "\\(").replace(")", "\\)");

            if (filterTextInput.value.length > 0) {
                filterTextInput.value += ".*";
            }
            filterTextInput.value += indexText;
        });
    }


});
