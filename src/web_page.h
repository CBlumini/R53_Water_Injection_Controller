
const char index_html[] PROGMEM = R"======(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }
    h2 {
      font-size: 3.0rem;
    }
    p {
      font-size: 3.0rem;
    }
    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px;
    } 
    .switch input {
      display: none;
    }
    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 34px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 68px;
    }
    input:checked + .slider {
      background-color: #2196F3;
    }
    input:checked + .slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px);
    }
  </style>
</head>
<body>
  <h3>Blumini Injects Monitor and Test Page</h3>
  %BUTTONPLACEHOLDER%
  <input type="text" id="var1" placeholder="Enter value for Injection Start RPM">
  <input type="text" id="var2" placeholder="Enter value for Injection End RPM">
  <button onclick="updateVariables()">Update Variables</button>
  <script>
    function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        xhr.open("GET", "/update?relay=" + element.id + "&state=1", true);
      } else {
        xhr.open("GET", "/update?relay=" + element.id + "&state=0", true);
      }
      xhr.send();
    }
    function updateVariables() {
      var var1 = document.getElementById('var1').value;
      var var2 = document.getElementById('var2').value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/updateVariables?var1=" + var1 + "&var2=" + var2, true);
      xhr.send();
    }
  </script>
</body>
</html>
)======";