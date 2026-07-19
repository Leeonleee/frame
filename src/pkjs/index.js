// Phone-side logic for the frame app.
//
// Provides the settings screen shown in the Pebble phone app (Settings ->
// frame). The page is a small self-contained HTML form delivered as a data:
// URI, so nothing needs to be hosted. When the user saves, the chosen values
// are stored on the phone and forwarded to the watch over AppMessage.

// Date-format options. The order must match s_date_formats in settings.c.
var DATE_FORMATS = ['19 Jul', 'Jul 19', '19/07/26'];

var DEFAULTS = { vibrate: true, dateFormat: 0 };

function loadSettings() {
  try {
    var stored = JSON.parse(localStorage.getItem('settings'));
    return {
      vibrate: stored.vibrate !== undefined ? stored.vibrate : DEFAULTS.vibrate,
      dateFormat: stored.dateFormat !== undefined ? stored.dateFormat
                                                   : DEFAULTS.dateFormat
    };
  } catch (e) {
    return { vibrate: DEFAULTS.vibrate, dateFormat: DEFAULTS.dateFormat };
  }
}

function buildConfigPage(settings) {
  var options = '';
  for (var i = 0; i < DATE_FORMATS.length; i++) {
    options += '<option value="' + i + '"' +
               (i === settings.dateFormat ? ' selected' : '') + '>' +
               DATE_FORMATS[i] + '</option>';
  }

  return [
    '<!DOCTYPE html><html><head>',
    '<meta name="viewport" content="width=device-width,initial-scale=1">',
    '<title>frame settings</title>',
    '<style>',
    'body{font-family:sans-serif;margin:0;background:#f4f4f4;color:#222}',
    'header{background:#008e8e;color:#fff;padding:18px 16px;font-size:22px}',
    '.row{display:flex;justify-content:space-between;align-items:center;',
    'background:#fff;padding:16px;border-bottom:1px solid #e0e0e0;font-size:17px}',
    'select{font-size:16px;padding:6px}',
    'input[type=checkbox]{transform:scale(1.6)}',
    'button{width:100%;padding:16px;font-size:18px;border:0;color:#fff;',
    'background:#ff8500;margin-top:24px}',
    '</style></head><body>',
    '<header>frame settings</header>',
    '<label class="row"><span>Vibrate on save</span>',
    '<input type="checkbox" id="vibrate"' + (settings.vibrate ? ' checked' : '') +
    '></label>',
    '<label class="row"><span>Date format</span>',
    '<select id="dateFormat">' + options + '</select></label>',
    '<button id="save">Save</button>',
    '<script>',
    'document.getElementById("save").addEventListener("click",function(){',
    'var data={vibrate:document.getElementById("vibrate").checked,',
    'dateFormat:parseInt(document.getElementById("dateFormat").value,10)};',
    'location.href="pebblejs://close#"+encodeURIComponent(JSON.stringify(data));',
    '});',
    '</script></body></html>'
  ].join('');
}

Pebble.addEventListener('showConfiguration', function() {
  var html = buildConfigPage(loadSettings());
  Pebble.openURL('data:text/html,' + encodeURIComponent(html));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) {
    return;  // user cancelled without saving
  }
  var settings;
  try {
    settings = JSON.parse(decodeURIComponent(e.response));
  } catch (err) {
    return;
  }
  localStorage.setItem('settings', JSON.stringify(settings));
  Pebble.sendAppMessage({
    'VIBRATE_ON_SAVE': settings.vibrate ? 1 : 0,
    'DATE_FORMAT': settings.dateFormat | 0
  });
});
