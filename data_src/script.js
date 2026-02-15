function send(code, name) {
  if (navigator.vibrate) {
    navigator.vibrate(100);
  }
  const status = document.getElementById("msg");
  status.innerText = "Envoi: 0x" + code;
  fetch("/api/ir?code=" + code + "&name=" + name)
    .then((r) => {
      if (r.ok) status.innerText = "Reçu par l'ESP";
    })
    .catch((e) => (status.innerText = "Erreur de connexion"));
}

const buttons = document.querySelectorAll("button");

buttons.forEach((button) => {
  button.addEventListener("click", () => {
    const code = button.getAttribute("code");
    const name = encodeURI(
      button.getAttribute("name") || button.innerText || "Unknown",
    );
    if (code) {
      send(code, name);
    } else {
      document.getElementById("msg").innerText =
        "No IR code for button " + button.innerText;
    }
  });
});

document.body.addEventListener(
  "click",
  function () {
    let elem = document.documentElement;

    if (elem.requestFullscreen) {
      elem.requestFullscreen();
    } else if (elem.webkitRequestFullscreen) {
      elem.webkitRequestFullscreen();
    } else if (elem.msRequestFullscreen) {
      elem.msRequestFullscreen();
    }
  },
  { once: true },
);
