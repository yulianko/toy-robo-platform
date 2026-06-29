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
            </script>
        </head>
        <body>
            <div class="container">
                <h2>Robot Control</h2>
                <div>
                    <button class="btn" onclick="sendCmd('control/api/command/move', {dir: 'forward'})">Forward</button>
                </div>
                <div>
                    <button class="btn" onclick="sendCmd('control/api/command/move', {dir: 'left'})">Left</button>
                    <button class="btn btn-stop" onclick="sendCmd('control/api/command/stop')">STOP</button>
                    <button class="btn" onclick="sendCmd('control/api/command/move', {dir: 'right'})">Right</button>
                </div>
                <div>
                    <button class="btn" onclick="sendCmd('control/api/command/move', {dir: 'backward'})">Backward</button>
                </div>
                <hr>
                <h3>Indicators</h3>
                <button class="btn btn-warn" onclick="sendCmd('control/api/command/indicator', {action: 'alert'})">Trigger Alert</button>
                <button class="btn" onclick="sendCmd('control/api/command/indicator', {action: 'stop'})">Clear Indicator</button>
            </div>
        </body>
        </html>
    )html";
}  // namespace RobotControlHttpPageContent
