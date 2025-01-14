

function setTableFilter(table, mode) {

    // Find outcome column
    let outcomeColumn = -1;

    const firstRow = table.rows[0].cells;
    for (let i = 0; i < firstRow.length; i++) {
        const text = firstRow[i].children[0].innerText;

        if (text === "Outcome") {
            outcomeColumn = i;
            break;
        }
    }

    if (outcomeColumn == -1) {
        console.log("Failed to find outcome column index");
        return;
    }

    // Iterate over rows
    for (let i = 1; i < table.rows.length; i++) {
        const row = table.rows[i];
        const text = row.cells[outcomeColumn].children[0].innerText;

        row.hidden = false;

        if (mode == "all") {
            row.hidden = false;
        } else if (mode == "bad" && !(text == "FAIL" || text == "TIMEOUT")) {
            row.hidden = true;
        } else if (mode == "timeout" && !(text == "TIMEOUT")) {
            row.hidden = true;
        } else if (mode == "fail" && !(text == "FAIL")) {
            row.hidden = true;
        }
        
    }
}


document.addEventListener("DOMContentLoaded", () => {
    const dropdown = document.getElementById("outcome-filter-dropdown");
    const table = document.getElementById("data-table");

    dropdown.addEventListener("change", (e) => {
        const value = e.target.value;
        setTableFilter(table, value);
    });

    setTableFilter(table, dropdown.value);

});
