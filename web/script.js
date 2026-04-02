async function getData() {
  const res = await fetch("/data");
  const data = await res.json();

  document.getElementById("pulse").textContent = data.pulse;
}

setInterval(getData, 1000);
