// Row filtering settings
let g_mode = "";
let g_filterText = "";
let g_include = true;
let g_regex = false;
let g_searchColumn = -1;
let g_columnVisibility = [];
let g_radioSortFn = null;

const initialRowOrder = [];

function hasColumn(colName) {
    return pyvar_columnIndices.hasOwnProperty(colName);
}

function getColumnIdx(colName) {
    const idx = pyvar_columnIndices[colName];

    if (typeof(idx) !== "number")
        console.error(`Tried to get index for non-existent column ${colName}`);

    return idx;
}

function showTable() {
    const table = document.getElementById("data-table");
    table.style.visibility = "visible";
}

function showHideIndices() {
    const indexRow = document.getElementById("col-indices");

    if (g_searchColumn != -2) {
        indexRow.hidden = true;
    } else {
        indexRow.hidden = false;
    }
}

// uses data-num HTML attribute, NOT the cell's text
function getSortFn_num(columnName, greaterFirst) {
    const columnIdx = getColumnIdx(columnName)

    return (row1, row2) => {
        const data1 = row1.cells[columnIdx].dataset["num"];
        const data2 = row2.cells[columnIdx].dataset["num"];

        const time1 = Number.parseFloat(data1);
        const time2 = Number.parseFloat(data2);

        let ordinal;

        if (isNaN(time1) && isNaN(time2))
            return 0;

        if (time1 == time2)
            return 0;

        if (isNaN(time1))
            ordinal = -1;
        else if (isNaN(time2))
            ordinal =  1;
        else
            ordinal = time1 < time2 ? -1 : 1;

        if (greaterFirst)
            return -ordinal;
        return ordinal;
    };
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
        showByClass(["row-diverging-result"], true);
        showByClass(["row-timeout"], true);
        showByClass(["row-bad-hash"], true);
    }

    if (g_mode == "fail") {
        showByClass(["row"], false);
        showByClass(["row-fail"], true);
    }

    if (g_mode == "diverging-result") {
        showByClass(["row"], false);
        showByClass(["row-diverging-result"], true);
    }

    if (g_mode == "timeout") {
        showByClass(["row"], false);
        showByClass(["row-timeout"], true);
    }

    if (g_mode == "hash") {
        showByClass(["row"], false);
        showByClass(["row-bad-hash"], true);
    }

    if (g_mode == "duplicate") {
        showByClass(["row"], false);
        showByClass(["row-duplicate"], true);
    }

    if (g_mode == "no-problem") {
        showByClass(["row"], true);
        showByClass(["row-fail"], false);
        showByClass(["row-timeout"], false);
        showByClass(["row-bad-hash"], false);
        showByClass(["row-diverging-result"], false);
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

function sortTableByFn() {
    const table = document.getElementById("data-table");
    const tableRows = table.rows;

    const dataRows = [];

    const tableRowsLength = tableRows.length;
    for (let i = 2; i < tableRowsLength; i++) {
        const row = tableRows[i];

        if (row.hidden)
            continue;

        dataRows.push(row);
    }

    dataRows.sort(g_radioSortFn);

    const dataRowsLength = dataRows.length;
    for (let i = 0; i < dataRowsLength; i++)
        table.appendChild(dataRows[i]);
}

function showHideTableColumns() {
    let rows = document.querySelectorAll(".row, .row-header");

    const columnCount = g_columnVisibility.length;
    for (let r of rows) {
        let children = r.children;
        for (let c = 0; c < columnCount; c++) {
            children[c].classList.toggle("hide-column", !g_columnVisibility[c]);
        }
    }
}

function sortTable() {
    if (g_radioSortFn === null) {
        const table = document.getElementById("data-table");

        const initialRowOrderLength = initialRowOrder.length;
        for (let i = 0; i < initialRowOrderLength; i++)
        {
            const row = initialRowOrder[i];
            if (row.hidden)
                continue;
            table.appendChild(initialRowOrder[i]);
        }
    } else {
        sortTableByFn();
    }
}

function handleSortRadioChange(new_val) {
    console.log("Changing sort key to " + new_val);
    switch (new_val) {
        case "none": {
            g_radioSortFn = null;
            break;
        }

        case "time": {
            g_radioSortFn = getSortFn_num("time", true);
            break;
        }

        case "node-count": {
            g_radioSortFn = getSortFn_num("node_count", true);
            break;
        }

        case "time-speedup": {
            g_radioSortFn = getSortFn_num("faster", true);
            break;
        }

        case "time-slowdown": {
            g_radioSortFn = getSortFn_num("faster", false);
            break;
        }

        case "node-count-speedup": {
            g_radioSortFn = getSortFn_num("node_faster", true);
            break;
        }

        case "node-count-slowdown": {
            g_radioSortFn = getSortFn_num("node_faster", false);
            break;
        }

        default: {
            console.log(`Unrecognized \"sort by\" function: \"${new_val}\"`);
            g_radioSortFn = sortFn_identity;
            break;
        }
    }
}

function deleteRadioOptions(radioIds) {
    for (const id of radioIds) {
        const element = document.querySelector(`#sort-radio-list li #${id}`);
        const parent = element.parentElement;
        console.log(parent);
        parent.remove();
    }
}

function initializeSortRadio() {
    // Delete radio options for non-existent columns

    if (!hasColumn("faster")) {
        deleteRadioOptions(["sort-radio-time-speedup", "sort-radio-time-slowdown"]);
    }

    if (!hasColumn("node_faster")) {
        deleteRadioOptions(["sort-radio-node-count-speedup", "sort-radio-node-count-slowdown"]);
    }

    const elements =
        document.querySelectorAll("#sort-radio-list li input[type=radio]");

    for (let element of elements) {
        element.addEventListener("change", (e) => {
            handleSortRadioChange(e.target.value);
            refresh();
        });

        // Set visible
        element.parentElement.classList.toggle("hide-radio", false);
    }

    const initialValue = document.querySelector("input[name=sort-radio-value]:checked").value;
    console.log(`Initial value is \"${initialValue}\"`);
    handleSortRadioChange(initialValue);



}

/*
    The bottleneck here is how long it takes the browser to re-render the table
        after the DOM changes.
*/
function refresh() {
    showTable();
    showHideIndices();
    setTableFilter();
    setTableFilterText();
    showHideTableColumns();
    sortTable();
}

// Wait for the document to finish loading before doing anything
document.addEventListener("DOMContentLoaded", () => {
    const table = document.getElementById("data-table");
    const tableRows = table.rows;
    const tableRowsLength = tableRows.length;
    for (let i = 2; i < tableRowsLength; i++)
        initialRowOrder.push(tableRows[i]);


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
            "row-css": "row-diverging-result",
            "main-text": "\"completed\" test(s) with diverging results",
        },

        {
            "row-css": "row-timeout",
            "new-css": "cell-new-timeout",
            "main-text": "test(s) timed out",
            "secondary-text": "newly timed out",
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

    // Set up column visibility
    const visCheckboxes = document.getElementsByClassName("vis-checkbox");
    const visCheckboxesLength = visCheckboxes.length;

    g_columnVisibility = Array(visCheckboxesLength).fill(true); 

    for (let i = 0; i < visCheckboxesLength; i++) {
        let checkbox = visCheckboxes[i];

        if (!checkbox.checked) {
            g_columnVisibility[i] = false;
        }

        checkbox.addEventListener("change", (e) => {
            g_columnVisibility[i] = e.target.checked;
            refresh();
        });
    }

    initializeSortRadio();

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
