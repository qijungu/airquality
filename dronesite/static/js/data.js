var datainterval = 1000;     // 1000 milliseconds
var dataerrcount = 0;
var datafeedtimeout = null;
var datacount = 0;

$(function() {
	datafeed();
});

function datafeed() {
	if (dataerrcount < 0) return;
	$.post('../datafeed/')
	.then(function(r){
		ret = $.parseJSON(r);
		dataerrcount = 0;
		if (ret[0]) {
			$("#alert .connected").css("display", "block");
			$("#alert .nodata").css("display", "none");
			$("#alert .noconnection").css("display", "none");
		} else {
			$("#alert .connected").css("display", "none");
			$("#alert .nodata").css("display", "block");
			$("#alert .noconnection").css("display", "none");
		}
		var data=ret[1];
		$(".info #status").text(data['status']);
		$(".info #mode").text(data['mode']);
		$(".info #battery").text(data['battery']+'%');
		$(".info #speed").text(data['airspeed'].toFixed(1));
		$(".info #time").text((data['ts']+data['tus']/1000000.0).toFixed(1));
		$(".info #altitude").text(data['altitude'].toFixed(2));
		$(".info #latitude").text(data['latitude'].toFixed(6));
		$(".info #longitude").text(data['longitude'].toFixed(6));
		var outtxt = datacount.toString() + ', '
					+ (data['ts']+data['tus']/1000000.0).toFixed(2).toString() + ', '
					+ data['pm1'].toFixed(3).toString() + ', '
					+ data['pm2'].toFixed(3).toString() + ', '
					+ data['pm10'].toFixed(3).toString() + ', '
					+ data['no2we'].toFixed(1).toString() + ', '
					+ data['no2ae'].toFixed(1).toString() + ', '
					+ data['altitude'].toFixed(3).toString() + ', '
					+ data['latitude'].toFixed(9).toString() + ', '
					+ data['longitude'].toFixed(9).toString();
		var outdiv = $("<div />");
		outdiv.text(outtxt);
		$(".datafeed").prepend(outdiv);
		var outdivs = $(".datafeed").children();
		if (outdivs.length > 30) {
			outdivs.last().remove();
		}
		datacount++;
		datafeedtimeout = setTimeout(datafeed, datainterval)
	}, function(r) {
		if (dataerrcount < 0) return;
		dataerrcount++;
		datafeedtimeout = setTimeout(datafeed, datainterval)
		if (dataerrcount > 20) {
			clearTimeout(datafeedtimeout);
			dataerrcount = -1;
			$("#alert .connected").css("display", "none");
			$("#alert .nodata").css("display", "none");
			$("#alert .noconnection").css("display", "block");
		}
	});
}

function stopdata() {
	window.stop();
}

