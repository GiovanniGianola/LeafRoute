console.log("Loading Handle Penalty");
let data = {
    change: 0,
    perc: 0,
    multi: 1
};



$(document).ready(function(){
    // HANDLE ON APPLY PENALTY
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
            data.perc = perc;
            data.multi = multi;
            postPenalty();
        }
   	});

	$("#defaultGraph").on("click", function() {
	    console.log('Use default graph');
        data.change = 0;
	    postPenalty();
        $('#multiplier').val('');
        $('#percentage').val('');
    });


});

// -------------- REQUESTS ---------------

function postPenalty(){
    
	console.log(endpoint);
	console.log(data);
    
    $.post(endpoint + '/postpenalty/', data, 'json').done(function(response) {
			console.log('Request Done: ' + response);
        }).fail(function(textStatus, error) {
            alert(textStatus.responseText);
            console.log('Request Failed: ' + textStatus.responseText + ', ' + textStatus.status);
        });
}
