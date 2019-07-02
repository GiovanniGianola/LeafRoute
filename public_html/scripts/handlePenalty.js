console.log("Loading Handle Penalty");
let data = {};
let func = ['add', 'del'];

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
    // HANDLE ON APPLY PENALTY
    /*
	$("#applyPenalty").on("click", function() {
        perc = parseInt($('#percentage').val());
        multi = parseInt($('#multiplier').val());
        console.log(perc);
        console.log(multi);
        var ok = true;

        if(Number.isNaN(perc) || perc < 0 || typeof perc != 'number' || perc > 100){
            alert("Invalid input: 0 <= Percentage <= 100");
            perc = 0;
            $('#percentage').val(0);
            ok = false;
        }
        if(Number.isNaN(multi) || multi < 1 || typeof multi != 'number' || multi > 10){
            alert("Invalid input: 1 <= Multiplier <= 10");
            multi = 1;
            $('#multiplier').val(1);
            ok = false;
        }
        if(ok){
            data.change = 1;
            data.multi = multi;
            postPenalty();
        }
   	});
    */
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

    map.on(L.Draw.Event.CREATED, function (e) {

        var layer = e.layer;
        drawnItems.addLayer(layer);
        data = layer.getLatLngs();

        fillData(data);
        postRect.function = func[0];
        postPenalty(postRect);

        console.log('OnCreated');
        console.log(layer.getLatLngs());

    });

    map.on('draw:deleted', function (e) {
        var layers = e.layers;
        layers.eachLayer(function (layer) {

            data = layer.getLatLngs();
            fillData(data);
            postRect.function = func[1];
            postPenalty(postRect);

            console.log('OnRemove');
            console.log(layer.getLatLngs())
        });
    });
});

// -------------- REQUESTS ---------------

function postPenalty(data){
    
	console.log(endpoint);
	console.log(data);

    $.post(endpoint + '/postpenalty/', data, 'json').done(function(response) {
            var json = JSON.parse(response);
			console.log('Request Done');
			console.log(json);
            drawPolylines(json);
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

// -------------- AUXILIARY FUNCTIONS ---------------

function fillData(data){

    var lat = [];
    var lng = [];

    for (i = 0; i < data[0]["length"]; i++) {
        lat.push(data[0][i].lat);
    }
    for(i=0; i<data[0]["length"]; i++) {
        lng.push(data[0][i].lng);
    }

    postRect.min_lat = Math.min( ...lat );
    postRect.max_lat = Math.max( ...lat );
    postRect.min_long = Math.min( ...lng );
    postRect.max_long = Math.max( ...lng );
}
