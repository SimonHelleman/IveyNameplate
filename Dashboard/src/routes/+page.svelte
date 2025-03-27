<script>
    import { onMount } from "svelte";
    import logo from "$lib/images/western-brand.svg";
	import { sequence } from "@sveltejs/kit/hooks";

    let studentsLogged = 0;
    let handsRaised = 0;
    let thumbsUp = 0;
    let thumbsDown = 0;

    let chart;

    let studentsInClass = [];

    let quietMode = "off"; // Default
    let anonymousMode = "off"; //Default

    async function setQuietMode(mode) {
        try {
            const resp = await fetch(`/nameplate/quietmode?mode=${mode}`, { method: "GET" });
            if (!resp.ok) throw new Error(`Error setting quiet mode! Status: ${resp.status}`);
            console.log(`Quiet mode set to: ${mode}`);
        } catch (error) {
            console.error("Error updating quiet mode:", error);
        }
    }
    
    async function setAnonymousMode(mode) {
        try {
            const resp = await fetch(`/nameplate/anonymous?mode=${mode}`, { method: "GET" });
            if (!resp.ok) throw new Error(`Error setting anonymous mode mode! Status: ${resp.status}`);
            console.log(`Anonymous mode set to: ${mode}`);
        } catch (error) {
            console.error("Error updating anonymous mode:", error);
        }
    }

    function handleQuietModeChange(event) {
        quietMode = event.target.value; // Update variable
        setQuietMode(quietMode); // Send update to API
    }

    function handleAnonymousModeChange(event) {
        anonymousMode = event.target.value;
        setAnonymousMode(anonymousMode);
    }

    async function onClear(event) {
        console.log("onClear()");
        const resp = await fetch(`/nameplate/clearreact`, { method: "GET" });
        if (!resp.ok) throw new Error(`Error clearing reactions! Status: ${resp.status}`);
    }

    onMount(async () => {
        const Chart = (await import("chart.js/auto")).default;
        const ctx = document.getElementById("responseChart").getContext("2d");

        const pollDataResp = await fetch("/nameplate/polldata");
        const studentDataResp = await fetch("/nameplate/studentsinclass");
        const reactionResp = await fetch("/nameplate/reactions");
        
        
        if (!pollDataResp.ok) {
            throw new Error(`HTTP error! Status: ${pollDataResp.status}`);
        }
        
        if (!studentDataResp.ok) {
            throw new Error(`HTTP error! Status: ${studentDataResp.status}`);
        }
        if (!reactionResp.ok) {
            throw new Error(`HTTP error! Status: ${reactionResp.status}`);
        } 
        
        const pollData = await pollDataResp.json();
        const studentData = await studentDataResp.json();
        const reactionData = await reactionResp.json();

        studentsInClass = studentData.data.map(({ nameLast, nameFirst }) => `${nameLast} ${nameFirst}`);

        handsRaised = reactionData.handsUp;
        thumbsUp = reactionData.thumbsUp;
        thumbsDown = reactionData.thumbsDown;

        studentsLogged = studentsInClass.length;

        await setQuietMode("off");
        await setAnonymousMode("off");

        const dataArray = pollData.data;

        console.log(dataArray);

        chart = new Chart(ctx, {
            type: "bar",
            data: {
                labels: dataArray.map(d => d.option),
                datasets: [{
                    data: dataArray.map(d => d.count),
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
        <select value={quietMode} on:change={handleQuietModeChange}>
            <option value="on">On</option>
            <option value="off">Off</option>
            <option value="partial">Partial</option>
        </select>
    </label>
    <label>Anonymous Mode:
        <select value={anonymousMode} on:change={handleAnonymousModeChange}>
            <option value="off">Off</option>
            <option value="on">On</option>
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
        <p>Thumbs Up: {thumbsUp}</p>
        <p>Thumbs Down: {thumbsDown}</p>
    </div>
    <div class="section">
        <h3>Student List</h3>
        <ul>
            {#each studentsInClass as student}
                <li>{student}</li>
            {/each}
        </ul>
    </div>
    <div class="section">
        <!--
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
        -->
    </div>
    <div class="section">
        <canvas id="responseChart"></canvas>
    </div>
</div>

<div class="footer">
    <button on:click={onClear}>Clear</button>
    <button on:click={() => console.log("Help button clicked")}>Help</button>
</div>
