const char homePage[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>First Aid Monitor</title>

    <style>
      body {
        background-color: #e6f4ea;
        font-family: Arial, sans-serif;
        text-align: center;
        margin-top: 40px;
      }

      h1 {
        color: darkgreen;
      }

      .card {
        background: white;
        padding: 20px;
        margin: 10px auto;
        width: 280px;
        border-radius: 10px;
        box-shadow: 0px 2px 8px rgba(0,0,0,0.2);
      }

      .value {
        font-size: 28px;
        color: red;
        font-weight: bold;
      }
    </style>
  </head>

  <body>
    <h1>Patient Monitor</h1>

    <div class="card">
      <p>Pulse</p>
      <div id="pulse" class="value">--</div>
    </div>

    <div class="card">
      <p>SpO₂</p>
      <div id="spo2" class="value">--</div>
    </div>

    <div class="card">
      <p>Location</p>
      <div id="location" class="value">--</div>
    </div>

    <script>
      async function updateSensors() {
        try {
          const response = await fetch("/sensors");

          if (!response.ok) {
            throw new Error("HTTP error: " + response.status);
          }

          const data = await response.json();

          document.getElementById("pulse").textContent = data.pulse + " bpm";
          document.getElementById("spo2").textContent = data.spo2 + " %";
          document.getElementById("location").textContent =
            data.lat + ", " + data.lon;

        } catch (error) {
          console.log("Error:", error);

          document.getElementById("pulse").textContent = "ERR";
          document.getElementById("spo2").textContent = "ERR";
          document.getElementById("location").textContent = "ERR";
        }
      }

      updateSensors();
      setInterval(updateSensors, 2000);
    </script>

  </body>
</html>
)rawliteral";