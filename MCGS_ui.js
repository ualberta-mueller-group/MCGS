
function addButtons() {
    const b = document.createElement("button");
    b.innerHTML = "Download CSV (if exists)";


    b.addEventListener("click", () => {
        const data = Module.FS.readFile("out.csv");

        const blob = new Blob([data], {type: "text/plain"});

        const url = URL.createObjectURL(blob);

        const dl = document.createElement("a");
        dl.href = url;
        dl.download = "out.csv";
        dl.click();

        console.log("Added");

        setTimeout(() => {
            URL.revokeObjectURL(url);
        }, 1000);

    });

    document.body.appendChild(b);
}

document.addEventListener("DOMContentLoaded", addButtons);
