// Row filtering settings
let g_mode = "";
let g_filterText = "";
let g_include = true;
let g_regex = false;
let g_searchColumn = -1;
let g_columnVisibility = [];
let g_radioSortFn = null;
let g_regressionsFirst = false;

const initialRowOrder = [];

const regressionOrdinalMap = {
    "NEW FAIL": 0,
    "NEW TIMEOUT": 1,
    "NEW COMPLETED": 2,
    "NEW PASS": 3,

    "STILL FAIL": 4,
    "STILL TIMEOUT": 4,
    "STILL COMPLETED": 4,
    "STILL PASS": 4,
    "N/A": 4,
};

function getRegressionOrdinal(regressionCellText) {
    const ordinal = regressionOrdinalMap[regressionCellText];

    if (typeof(ordinal) !== "number") {
        console.error(`Couldn't get regression ordinal for regression cell text ${regressionCellText}`);
        return regressionOrdinalMap["N/A"];
    }

    return ordinal;
}

function divMod(top, bottom) {
    return [Math.floor(top / bottom), top % bottom];
}

function percentString(val1, val2) {
    const larger = Math.max(val1, val2);
    const smaller = Math.min(val1, val2);
    return (100.0 * (larger / smaller)).toFixed(2) + "%";
}

function multiplierString(val1, val2) {
    const larger = Math.max(val1, val2);
    const smaller = Math.min(val1, val2);
    return (larger / smaller).toFixed(2) + "x";
}

function toFixedNoTrailing0(num, nDigits) {
    num = num.toFixed(nDigits);

    if (nDigits === 0)
        return num;

    if (num.length === 0)
        return num;

    let popCount = 0;

    for (let idx = num.length - 1; idx >= 0; idx--) {
        const c = num[idx];

        if (c === "0") {
            popCount++;
            continue;
        }

        if (c === ".") {
            popCount++;
            break;
        }

        break;
    }

    return num.substr(0, num.length - popCount);
}

function commaSeparatedInt(num) {
    num = Math.floor(num).toString();

    const chunks = [];

    const numLength = num.length;
    for (let idx = numLength; idx > 0; ) {
        const chunkLength = Math.min(3, idx);
        chunks.push(num.substr(idx - chunkLength, chunkLength));

        idx -= chunkLength;
    }

    chunks.reverse();
    return chunks.join(",");
}

function timeMsToTimeObj(timeMs) {
    if (typeof(timeMs) !== "number")
        console.error(`timeMsToTimeObj argument not a number: "${timeMs}"`);

    const timeObj = {
        ms: timeMs,
        s: 0,
        m: 0,
        h: 0,
        d: 0,
    };

    if (timeObj.ms < 0) {
        console.error(`timeMsToTimeObj argument negative: "${timeMs}"`);
        return timeObj;
    }

    // To seconds?
    if (timeObj.ms < 1000.0)
        return timeObj;
    [timeObj.s, timeObj.ms] = divMod(timeObj.ms, 1000.0);

    // To minutes?
    if (timeObj.s < 60.0)
        return timeObj;
    [timeObj.m, timeObj.s] = divMod(timeObj.s, 60.0);

    // To hours?
    if (timeObj.m < 60)
        return timeObj;
    [timeObj.h, timeObj.m] = divMod(timeObj.h, 60.0);

    // To days?
    if (timeObj.h < 24)
        return timeObj;
    [timeObj.d, timeObj.h] = divMod(timeObj.h, 24.0);

    return timeObj;
}

function timeObjToString(timeObj) {
    const props = ["ms", "s", "m", "h", "d"];

    let error = false;
    for (const p of props) {
        if (!timeObj.hasOwnProperty(p)) {
            error = true;
            break;
        }
    }

    if (error)
        console.error(`timeObjToString argument missing field ${timeObj}`);

    let result = [];

    const addProp = (val, name, format) => {
        //const pluralText = val > 1 ? "s" : "";
        const valFmt = format ? Math.floor(val) : val;

        if (val > 0)
            result.push(`${valFmt}${name}`);
    };

    addProp(timeObj.d, "d", false);
    addProp(timeObj.h, "h", false);
    addProp(timeObj.m, "m", false);
    addProp(timeObj.s, "s", false);
    addProp(timeObj.ms, "ms", true);

    return result.join(", ");
}

function formatTimeMs(timeMs) {
    return timeObjToString(timeMsToTimeObj(timeMs));
}

function setAggregationText(tableRows) {
    let rows_hidden = 0;
    let rows_shown = 0;

    // Time
    let time_total = 0;
    const time_idx = getColumnIdx("time");

    // Node count
    let node_count_total = 0;
    const node_count_idx = getColumnIdx("node_count");

    // Old time
    let old_time_total = 0;
    const has_old_time = hasColumn("old_time");
    const old_time_idx = has_old_time ? getColumnIdx("old_time") : undefined;

    // Old node count
    let old_node_count_total = 0;
    const has_old_node_count = hasColumn("old_node_count");
    const old_node_count_idx = has_old_node_count ?
        getColumnIdx("old_node_count") : undefined;

    const tableRowsLength = tableRows.length;
    for (let i = 2; i < tableRowsLength; i++) {
        const row = tableRows[i];
        const cells = row.cells;

        if (row.hidden) {
            rows_hidden++;
            continue;
        }

        rows_shown++;

        // Time
        const time = parseFloat(cells[time_idx].dataset["num"])
        time_total += time;

        // Node count
        const node_count = parseFloat(cells[node_count_idx].dataset["num"])
        node_count_total += node_count;

        if (has_old_time) {
            const old_time = parseFloat(cells[old_time_idx].dataset["num"])
            old_time_total += old_time;
        }

        if (has_old_node_count) {
            const old_node_count = parseFloat(cells[old_node_count_idx].dataset["num"])
            old_node_count_total += old_node_count;
        }
    }

    const fmt0 = (num) => {
        return commaSeparatedInt(num);
    };

    const fmtMs = (num) => {
        return formatTimeMs(num);
    };

    const getNodeRate = (total_nodes, total_ms) => {
        return 1000.0 * (total_nodes / total_ms);
    };

    const getFasterPhrase = (isBetter) => {
        return isBetter ? "FASTER BY" : "SLOWER BY";
    };

    const getAsFastPhrase = (isBetter) => {
        return isBetter ? "AS FAST" : "AS SLOW";
    };

    const getAsFewPhrase = (isBetter) => {
        return isBetter ? "AS FEW" : "AS MANY";
    };


    const lines = [];

    const rows_total = rows_shown + rows_hidden;

    // Row counts
    lines.push([
        "👀",
        `${fmt0(rows_shown)} of ${fmt0(rows_total)} rows shown (${fmt0(rows_hidden)} hidden)`
    ]);

    // Total time
    lines.push([
        "⏳",
        `Total time: ${fmt0(time_total)} ms ＝ ${fmtMs(time_total)}`
    ]);

    // Total nodes
    lines.push([
        "🧮",
        `Total nodes: ${fmt0(node_count_total)} nodes`
    ]);

    // Node rate
    lines.push([
        "🚀",
        `Node rate (nodes/second): ${fmt0(1000 * (node_count_total / time_total))} n/s`
    ]);
    

    // Compare time
    if (has_old_time) {
        let timeDiff = time_total - old_time_total;
        const isBetter = timeDiff <= 0;
        timeDiff = Math.abs(timeDiff);

        const fasterPhrase = getFasterPhrase(isBetter);
        const asFastPhrase = getAsFastPhrase(isBetter);

        const percent = multiplierString(time_total, old_time_total);

        const clause1 = `${percent} ${asFastPhrase}`;
        const clause2 = `${fasterPhrase}: ${fmt0(timeDiff)} ms ＝ ${fmtMs(timeDiff)}`;
        const clause3 = `New: ${fmt0(time_total)} ms ← Old: ${fmt0(old_time_total)} ms`;

        lines.push([
            "🔎⏳",
            `Compared total time: (${clause1}) — (${clause2}) — (${clause3})`
        ]);
    }

    // Compare nodes
    if (has_old_node_count) {
        let nodeDiff = node_count_total - old_node_count_total;
        const isBetter = nodeDiff <= 0;
        nodeDiff = Math.abs(nodeDiff);

        const fasterPhrase = getFasterPhrase(isBetter);
        const asFewPhrase = getAsFewPhrase(isBetter);

        const percent = multiplierString(node_count_total, old_node_count_total);

        const clause1 = `${percent} ${asFewPhrase}`;
        const clause2 = `${fasterPhrase}: ${fmt0(nodeDiff)} nodes`;
        const clause3 = `New: ${fmt0(node_count_total)} nodes ← Old: ${fmt0(old_node_count_total)} nodes`;


        lines.push([
            "🔎🧮",
            `Compared node count: (${clause1}) — (${clause2}) — (${clause3})`
        ]);
    }

    // Compare node rate
    if (has_old_time && has_old_node_count) {
        const old_rate = getNodeRate(old_node_count_total, old_time_total);
        const new_rate = getNodeRate(node_count_total, time_total);


        let rateDiff = new_rate - old_rate;
        const isBetter = rateDiff >= 0;
        rateDiff = Math.abs(rateDiff);

        const fasterPhrase = getFasterPhrase(isBetter);
        const asFastPhrase = getAsFastPhrase(isBetter);

        const percent = multiplierString(new_rate, old_rate);

        const clause1 = `${percent} ${asFastPhrase}`;
        const clause2 = `${fasterPhrase}: ${fmt0(rateDiff)} n/s`;
        const clause3 = `New: ${fmt0(new_rate)} n/s ← Old: ${fmt0(old_rate)} n/s`;

        lines.push([
            "🔎🚀",
            `Compared node rate: (${clause1}) — (${clause2}) — (${clause3})`
        ]);
    }

    const aggregationList = document.getElementById("aggregation-list");

    // Clear elements
    while (aggregationList.firstChild)
        aggregationList.removeChild(aggregationList.firstChild);

    // Make new elements
    for (const linePair of lines) {
        const [emoji, text] = linePair;
        const lineElement = document.createElement("li");

        const emojiElement = document.createElement("span");
        emojiElement.innerHTML = emoji;
        emojiElement.classList.toggle("emoji-shadow", true);

        const textElement = document.createElement("span");
        textElement.innerHTML = " " + text;

        lineElement.appendChild(emojiElement);
        lineElement.appendChild(textElement);
        aggregationList.appendChild(lineElement);
    }
}

function getSortFn_regression() {
    const regressionIdx = getColumnIdx("regression");

    return (row1, row2) => {
        const row1Text = row1.cells[regressionIdx].innerText;
        const row2Text = row2.cells[regressionIdx].innerText;

        const ordinal1 = getRegressionOrdinal(row1Text);
        const ordinal2 = getRegressionOrdinal(row2Text);

        if (ordinal1 == ordinal2)
            return 0;

        return ordinal1 < ordinal2 ? -1 : 1;
    };
}

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

function sortTableByFn(sortFn) {
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

    dataRows.sort(sortFn);

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

function restoreInitialRowOrder() {
    const table = document.getElementById("data-table");

    const initialRowOrderLength = initialRowOrder.length;
    for (let i = 0; i < initialRowOrderLength; i++)
    {
        const row = initialRowOrder[i];
        if (row.hidden)
            continue;
        table.appendChild(initialRowOrder[i]);
    }
}

function getGlobalSortFn() {
    if (!g_regressionsFirst) {
        if (g_radioSortFn !== null)
            return g_radioSortFn;
        return null;
    }

    const regressionFn = getSortFn_regression();

    if (g_radioSortFn === null)
        return regressionFn;

    return (row1, row2) => {
        const regressionOrder = regressionFn(row1, row2);

        if (regressionOrder !== 0)
            return regressionOrder;

        return g_radioSortFn(row1, row2);
    };
}

function sortTable() {
    const sortFn = getGlobalSortFn();

    if (sortFn === null) {
        restoreInitialRowOrder();
    } else {
        sortTableByFn(sortFn);
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

    setAggregationText(document.getElementById("data-table").rows);
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
    const regressionsFirst = document.getElementById("regressions-first-checkbox");

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
    g_regressionsFirst = regressionsFirst.checked;

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

    if (hasColumn("regression")) {
        regressionsFirst.parentNode.classList.toggle("hide-radio", false);

        regressionsFirst.addEventListener("change", (e) => {
            g_regressionsFirst = e.target.checked;
            refresh();
        });

    } else {
        regressionsFirst.parentNode.remove();
    }

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
