import React, { useState, useEffect } from "react";
import axios from "axios";
import "./App.css";

const App = () => {
  const [currentTime, setCurrentTime] = useState("");
  const [alarmTime, setAlarmTime] = useState("");
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const esp32IP = "http://192.168.29.13";

  useEffect(() => {
    const interval = setInterval(() => {
      const now = new Date();
      const indianTime = new Intl.DateTimeFormat("en-US", {
        timeZone: "Asia/Kolkata",
        hour12: false,
        hour: "numeric",
        minute: "numeric",
        second: "numeric",
      }).format(now);
      setCurrentTime(indianTime);
    }, 1000);

    axios
      .get(`${esp32IP}/temp`)
      .then((response) => {
        setTemperature(response.data.temp);
        setHumidity(response.data.humidity);
      })
      .catch((error) => {
        console.error("Error fetching temperature and humidity:", error);
      });

    return () => clearInterval(interval);
  }, []);

  const setAlarm = async () => {
    try {
      const currentMillis = Date.now();
      const alarmMillis = new Date(alarmTime).getTime();
      const delayMillis = alarmMillis - currentMillis;

      const response = await axios.post(`${esp32IP}/alarm`, null, {
        params: {
          delayMillis,
          alarmTimestamp: alarmMillis,
        },
      });

      console.log("Alarm response:", response.data);
    } catch (error) {
      console.error("Error setting alarm:", error);
    }
  };

  const turnOnLED = async () => {
    try {
      const response = await axios.post(`${esp32IP}/turnOnLED`);
      console.log("Turn on LED response:", response.data);
    } catch (error) {
      console.error("Error turning on LED:", error);
    }
  };

  const turnOffLED = async () => {
    try {
      const response = await axios.post(`${esp32IP}/turnOffLED`);
      console.log("Turn off LED response:", response.data);
    } catch (error) {
      console.error("Error turning off LED:", error);
    }
  };

  const formattedTemperature = temperature
    ? temperature.toFixed(2)
    : "Loading...";

  return (
    <div className="App">
      <header className="App-header">
        <h1>ESP32 Home Automation Control</h1>
        <div className="container">
          <div className="alarm-section">
            <div className="card">
              <h2>Current Time (IST): {currentTime}</h2>
            </div>
            <div className="card alarm-container">
              <div className="alarm-setter">
                <label>Set Alarm Time (IST): </label>
                <input
                  type="datetime-local"
                  value={alarmTime}
                  onChange={(e) => setAlarmTime(e.target.value)}
                />
                <button onClick={setAlarm}>Set Alarm</button>
              </div>
              <div className="immediate-alarm">
                <button onClick={turnOnLED}>Turn On LED</button>
              </div>
              <div className="turn-off">
                <button onClick={turnOffLED}>Turn Off LED</button>
              </div>
            </div>
          </div>
          <div className="sensor-section">
            <div className="card sensor-readings">
              <h2>
                Temperature: {formattedTemperature} °C{" "}
                {/* Change applied here */}
              </h2>
              <h2>Humidity: {humidity ? `${humidity} %` : "Loading..."}</h2>
            </div>
          </div>
        </div>
      </header>
    </div>
  );
};

export default App;
