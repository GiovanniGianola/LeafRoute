console.log("Loading Handle Penalty");
let data = {};
let func = ['add', 'del'];
let multi = 2;
let penaltiesJson = {};

var drawControl;
let penalizedRoutesPolyline = [];

let postRect = {
    id: 0,
    multi: 2,
    function: func[0],
    min_lat: 0.0,
    max_lat: 0.0,
    min_long: 0.0,
    max_long: 0.0
};

$(document).ready(function(){
    L.EditToolbar.Delete.include({
        removeAllLayers: true
    });
    $("#set").click(function() {
        if(!drawControl) {
            drawControl = new L.Control.Draw({
                draw: {
                    polygon: false,
                    polyline: false,
                    marker: false,
                    circle: false,
                    rectangle:{
                        color: 'red'
                    }
                },
                edit: {
                    featureGroup: drawnItems,
                    edit: false,
                    delete: {
                        removeAllLayers: true
                    }
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
        if(map.hasLayer(drawnItems)){
            drawnItems.eachLayer(function(l){
                    data = l.getLatLngs();
                    fillData(data);
                    postRect.function = func[1];
                    postPenalty(postRect);
                    drawnItems.removeLayer(l);
                });
        }
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
            drawnItems.clearLayers();
            getRects();
        }).fail(function(textStatus, error) {
            alert(textStatus.responseText);
            console.log('Request Failed: ' + textStatus.responseText + ', ' + textStatus.status);
        });
}

function getRects(){
    $.getJSON(endpoint + '/getrects/').done(function(response) {
        penaltiesJson = response;
        console.log('Request Done');
        drawRect(penaltiesJson);
        addPenaltyInfoPanel(penaltiesJson);
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

function drawRect(penaltiesJson){
    for(let i = 0; i < penaltiesJson.length; i++){
        let el = penaltiesJson[i];
        let pol = L.polygon([
            [el[2], el[3]],
            [el[2], el[5]],
            [el[4], el[5]],
            [el[4], el[3]]
        ]).setStyle({
            color: '#e98fb3'
        }).addTo(map);
        drawnItems.addLayer(pol);
    }
}

function addPenaltyInfoPanel(penaltiesJson){
    let htmlInfoPanel = ``;
    for(let i = 0; i < penaltiesJson.length; i++){
        let el = penaltiesJson[i];
        htmlInfoPanel += `
                            <div class="flex-container-info secondary-option col-12">
                                <p style="margin-top: 15px">Id: ${el[0]}</p>
                                <p style="margin-top: 15px">Multi: ${el[1]}</p>
                                <p style="margin-top: 15px">(${el[2].toFixed(3)},</p>
                                <p style="margin-top: 15px">${el[3].toFixed(3)})</p>
                                <p style="margin-top: 15px">(${el[4].toFixed(3)},</p>
                                <p style="margin-top: 15px">${el[5].toFixed(3)})</p>
                            </div>
                        `
    }
    $('#penalties-panel').html(htmlInfoPanel);
}

// -------------- AUXILIARY FUNCTIONS ---------------

function fillData(data){

    let lat = [];
    let lng = [];
    multi = parseInt($('#multiplier').val());

    for(let i = 0; i < data[0]["length"]; i++){
        lat.push(data[0][i].lat);
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
