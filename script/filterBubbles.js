var THRESH = 1.2;


fs = require('fs')
fs.readFile('bubbles.ms', 'utf8', function (err,data) {
  if (err) {
    return console.log(err);
  }

  var totalBubbles = 0;
  var killMode = false;
  var filteredData = [];

  function processLine(line) {
    var declarationResult = /^s\d+\s=.*radius:([\d\.]+)/.exec(line);
    if (declarationResult) {
      var radius = parseFloat(declarationResult[1]);
      killMode = (radius < THRESH);
      if (radius > THRESH) {
        totalBubbles++;
      }
    }
    if (!killMode) {
      filteredData += line + '\n';
    }
  }

  var start = 0;  
  var end = data.indexOf('\n', start);
  var i = 0;
  while (end !== -1) {
    var line = data.substring(start, end);
    processLine(line);
    // advance to next line
    start = end + 1;
    end = data.indexOf('\n', start);
    i++;
  }

  console.log(filteredData);
  console.log("total bubbles = " + totalBubbles);

  fs.writeFile("filtered.ms", filteredData, function(err) {
    if(err) {
        console.log(err);
    } else {
        console.log("The file was saved!");
    }
  }); 

});
