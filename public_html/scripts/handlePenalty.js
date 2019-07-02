console.log("Loading Handle Penalty");
let data = {};
let func = ['add', 'del'];
let multi = 2;
let rects = {};

var drawControl;
let penalizedRoutesPolyline = [];

let postRect = {
    multi: 2,
    function: func[0],
    min_lat: 0.0,
    max_lat: 0.0,
    min_long: 0.0,
    max_long: 0.0
};

$(document).ready(function(){
    $("#set").click(function() {
        if(!drawControl) {
            drawControl = new L.Control.Draw({
                draw: {
                    polygon: false,
                    polyline: false,
                    marker: false,
                    circle: false
                },
                edit: {
                    featureGroup: drawnItems,
                    edit: false
                }
            });
            map.addControl(drawControl);
            inSettings = true;
        }
    });
    $("#hom").click(function() {
        if(drawControl) {
            map.removeControl(drawControl);
            drawControl = undefined;
            inSettings = false;
        }
    });

    $("#defaultGraph").click(function() {
        map.removeLayer(drawnItems);
        getRects();
    });

    $("#sync_rect").click(function() {
        drawnItems.clearLayers();
        getRects();
    });

    map.on(L.Draw.Event.CREATED, function (e) {

        var layer = e.layer;
        drawnItems.addLayer(layer);
        data = layer.getLatLngs();

        fillData(data);
        postRect.function = func[0];
        postPenalty(postRect);
    });

    map.on('draw:deleted', function (e) {
        var layers = e.layers;
        layers.eachLayer(function (layer) {

            data = layer.getLatLngs();
            fillData(data);
            postRect.function = func[1];
            postPenalty(postRect);
        });
    });

    getRects();
});

// -------------- REQUESTS ---------------

function postPenalty(data){
    $.post(endpoint + '/postpenalty/', data, 'json').done(function(response) {
			console.log('Request Done: ' + response);
            //drawPolylines(json);
        }).fail(function(textStatus, error) {
            alert(textStatus.responseText);
            console.log('Request Failed: ' + textStatus.responseText + ', ' + textStatus.status);
        });
}

function getRects(){
    $.getJSON(endpoint + '/getrects/').done(function(response) {
        rects = response;
        console.log('Request Done');
        console.log(rects.length);
        drawRect(rects);
    }).fail(function(textStatus, error) {
        alert(textStatus.responseText);
        console.log('Request Failed: ' + textStatus.responseText + ', ' + textStatus.status);
    });
}

// -------------- HTML ---------------

function drawPolylines(json){
    var c_path = {};
    for (let i = json.length-1; i >= 0; i--){
        c_path = json[i];
        fancyPolyline = strokePolyline(
            c_path,
            {
                ...routeColors[i===0 ? 0 : 1],
                groupId: i
            });
        penalizedRoutesPolyline.push(fancyPolyline);
        fancyPolyline.addTo(map);
    }
}

function drawRect(json){
    for(i = 0; i < json.length; i++){
       var el = json[i];
       var pol = L.polygon([
           [el[0], el[1]],
           [el[0], el[3]],
           [el[2], el[3]],
           [el[2], el[1]]
        ]).setStyle({
           color: '#eea0be'
       }).addTo(map);
       drawnItems.addLayer(pol);
    }
}

// -------------- AUXILIARY FUNCTIONS ---------------

function fillData(data){

    var lat = [];
    var lng = [];
    multi = parseInt($('#multiplier').val());

    for (i = 0; i < data[0]["length"]; i++) {
        lat.push(data[0][i].lat);
    }
    for(i=0; i<data[0]["length"]; i++) {
        lng.push(data[0][i].lng);
    }

    if(Number.isNaN(multi) || multi < 1 || typeof multi != 'number' || multi > 10){
        multi = 2;
        $('#multiplier').val(2);
    }

    postRect.min_lat = Math.min( ...lat );
    postRect.max_lat = Math.max( ...lat );
    postRect.min_long = Math.min( ...lng );
    postRect.max_long = Math.max( ...lng );
    postRect.multi = multi;
}
