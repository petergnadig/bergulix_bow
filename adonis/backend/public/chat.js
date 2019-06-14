let ws = null;
let samples = 1000;
$("#samples").val(samples);

$(function() {
  start();
});

function start() {
  ws = adonis.Ws().connect();

  ws.on("open", () => {
    $(".connection-status").addClass("connected");
    subscribeToChannel();
  });

  ws.on("error", () => {
    $(".connection-status").removeClass("connected");
  });
}

function subscribeToChannel() {
  const chat = ws.subscribe("measurement");

  chat.on("error", () => {
    $(".connection-status").removeClass("connected");
  });

  chat.on("message", msg => {
    $(".messages").append(`
      <div class="message"><h3> ${msg.sender} </h3>
      <p>${JSON.stringify(msg)}</p> </div>
    `);
  });
}

$("#samples").change(function(e) {
  samples = this.value;
});

$("#startButton").click(function(e) {
  ws.getSubscription("measurement").emit("message", {
    sender: "commander",
    phase: "START",
    samples: samples
  });
});
