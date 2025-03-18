<script>
    import { onMount } from "svelte";
    import logo from "$lib/images/western-brand.svg";

    let studentsLogged = 25;
    let handsRaised = 3;
    let uniqueHands = 10;

    let responseData = [
        { option: "A", count: 8, percentage: "32%" },
        { option: "B", count: 10, percentage: "40%" },
        { option: "C", count: 5, percentage: "20%" },
        { option: "D", count: 2, percentage: "8%" }
    ];

    let chart;

    onMount(async () => {
        const Chart = (await import("chart.js/auto")).default;
        const ctx = document.getElementById("responseChart").getContext("2d");

        chart = new Chart(ctx, {
            type: "bar",
            data: {
                labels: responseData.map(d => d.option),
                datasets: [{
                    data: responseData.map(d => d.count),
                    backgroundColor: ["red", "green", "orange", "blue"]
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: false }
                },
                scales: {
                    y: { beginAtZero: true }
                }
            }
        });
    });
</script>

<style>
    body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        height: 100vh;
        width: 100vw;
        overflow: hidden;
    }
    .top-stripe {
        background: #f7f7f7;
        height: 100px;
        width: 100%;
        position: relative;
        z-index: 10;
        padding: 12px;
    }
    .top-bar {
        display: flex;
        align-items: center;
        justify-content: space-between;
        background: #4F2683;
        color: white;
        padding: 20px;
    }
    .container {
        display: grid;
        grid-template-columns: 1fr 1fr;
        grid-template-rows: 1fr 2fr;
        flex-grow: 1;
        gap: 10px;
        padding: 10px;
        width: 99%;
        height: calc(100vh - 280px);
        overflow: hidden;
    }
    .section {
        border: 1px solid #f3f3f3;
        padding: 10px;
        background: #ffffff;
        overflow: auto;
    }
    .footer {
        display: flex;
        justify-content: space-between;
        gap: 20px;
        background: #4B0082;
        padding: 10px;
    }
    .footer button {
        background: white;
        color: black;
        border: none;
        padding: 10px 15px;
        cursor: pointer;
        font-size: 14px;
        border-radius: 5px;
    }
</style>

<div class="top-stripe">
    <img src={logo} alt="Western Logo" width = "200">
</div>

<div class="top-bar">
    <label>Quiet Mode:
        <select>
            <option value="on">On</option>
            <option value="off">Off</option>
            <option value="partial">Partial</option>
        </select>
    </label>
    <label>Anonymous Mode:
        <select>
            <option value="on">On</option>
            <option value="off">Off</option>
        </select>
    </label>
    <label>Font Size:
        <select>
            <option value="small">Small</option>
            <option value="medium" selected>Medium</option>
            <option value="big">Big</option>
        </select>
    </label>
</div>

<div class="container">
    <div class="section">
        <h3>Classroom Stats</h3>
        <p>Students Logged In: {studentsLogged}</p>
        <p>Current Hands Raised: {handsRaised}</p>
        <p>Unique Hands Raised: {uniqueHands}</p>
    </div>
    <div class="section">
        <h3>Student List</h3>
        <ul>
            <li>Student 1</li>
            <li>Student 2</li>
            <li>Student 3</li>
            <li>Student 4</li>
            <li>Student 5</li>
            <li>Student 6</li>
        </ul>
    </div>
    <div class="section">
        <h3>Student Responses</h3>
        <table>
            <thead>
                <tr>
                    <th>Option</th>
                    <th>Number of Students</th>
                    <th>Percentage</th>
                </tr>
            </thead>
            <tbody>
                {#each responseData as { option, count, percentage }}
                    <tr>
                        <td>{option}</td>
                        <td>{count}</td>
                        <td>{percentage}</td>
                    </tr>
                {/each}
            </tbody>
        </table>
    </div>
    <div class="section">
        <canvas id="responseChart"></canvas>
    </div>
</div>

<div class="footer">
    <button on:click={() => console.log("Clear button clicked")}>Clear</button>
    <button on:click={() => console.log("Help button clicked")}>Help</button>
</div>
