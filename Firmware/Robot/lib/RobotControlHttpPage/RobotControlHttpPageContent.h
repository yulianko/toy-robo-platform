namespace RobotControlHttpPageContent {
const char* CSS = R"css(
        body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; padding: 20px; }
        .btn { display: inline-block; padding: 15px 25px; margin: 10px; font-size: 18px; color: white; background: #007BFF; border: none; border-radius: 5px; cursor: pointer; }
        .btn-stop { background: #DC3545; }
        .btn-warn { background: #FFC107; color: black; }
        .container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
    )css";

const char* HTML_PAGE = R"html(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Robot Control Panel</title>
            <link rel="stylesheet" href="/control/style.css">
            <script>
                let currentSpeed = 0.65;
                async function sendCmd(url, body = {}) {
                    try {
                        let res = await fetch(url, {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify(body)
                        });
                        if(!res.ok) alert('Command failed');
                    } catch (e) {
                        alert('Network error');
                    }
                }
                function moveWithSpeed(dir) {
                    sendCmd('control/api/command/move', {dir: dir, speed: currentSpeed});
                }
            </script>
        </head>
        <body>
            <div class="container">
                <h2>Robot Control</h2>
                <div>
                    <label for="speedControl">Speed (0-1):</label>
                    <input type="range" id="speedControl" min="0" max="1" step="0.05" value="0.65" style="width: 200px;">
                    <span id="speedValue">0.65</span>
                    <script>
                        document.getElementById('speedControl').addEventListener('input', (e) => {
                            currentSpeed = parseFloat(e.target.value);
                            document.getElementById('speedValue').textContent = currentSpeed.toFixed(2);
                        });
                    </script>
                </div>
                <hr>
                <div>
                    <button class="btn" onclick="moveWithSpeed('forward')">Forward</button>
                </div>
                <div>
                    <button class="btn" onclick="moveWithSpeed('left')">Left</button>
                    <button class="btn btn-stop" onclick="sendCmd('control/api/command/stop')">STOP</button>
                    <button class="btn" onclick="moveWithSpeed('right')">Right</button>
                </div>
                <div>
                    <button class="btn" onclick="moveWithSpeed('pivot_left')">Pivot Left</button>
                    <button class="btn" onclick="moveWithSpeed('pivot_right')">Pivot Right</button>
                </div>
                <div>
                    <button class="btn" onclick="moveWithSpeed('backward')">Backward</button>
                </div>
                <hr>
                <h3>Indicators</h3>
                <div>
                    <select id="animationSelect" style="padding: 8px; font-size: 16px; margin-right: 10px;">
                        <option value="0">Exploring</option>
                        <option value="1" selected>Curiosity</option>
                        <option value="2">Surprise</option>
                        <option value="3">Agreement</option>
                        <option value="4">Disagreement</option>
                        <option value="5">Danger</option>
                    </select>
                    <button class="btn btn-warn" onclick="let anim = parseInt(document.getElementById('animationSelect').value); sendCmd('control/api/command/indicator', {action: 'play', animation: anim})">Trigger</button>
                    <button class="btn" onclick="sendCmd('control/api/command/indicator', {action: 'stop'})">Clear</button>
                </div>
            </div>
        </body>
        </html>
    )html";
}  // namespace RobotControlHttpPageContent
