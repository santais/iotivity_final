/*var express		= require("express");
var bodyParser  =        require("body-parser");
var app 		= express();

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.get('/', function (req, res){

	fs = require('fs');
	var img = fs.readFileSync('/home/markpovlsen/Pictures/nicolas.png');
	res.writeHead(200, {'Content-Type': 'image/png' });
	res.end(img, 'binary');
});

app.post('handle', function(req, res) {
	console.log("Received post request");
});

app.listen(8081);*/

/*
* Module dependencies.
*/
var express    = require('express')    
    , fs       = require('fs')
    , http     = require('http')
    , util     = require('util')
    , path     = require('path')
    , bodyParser= require('body-parser');

/*
 * Global Variables 
 */
var app = express();

var server = require('http').createServer(app);
var io     = require('socket.io')(server);

var dir = __dirname;

var header = 'Welcome to this page.';
var image_path = "nodejs.png";

// Sensor variables
var sensorReadings = [[0, 0]];
var latestSensorReading = 0.0;
var tempSensorFound = false;

// IoTivity button input
var updateGraph = 0b1;

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.use(express.static(__dirname + '/'));

// IoTivity Resource 
var intervalId,
	handleReceptacle = {},

	// This is the same value as server.get.js
	sampleUri = "/oic/graph",
	iotivity = require( "iotivity-node/lowlevel" );

/*
 * Constants
 */
const PORT 				= 8082;
const SAMPLE_FREQUENCY 	= 1; // HERTZ
const SAMPLE_TIME 		= (1 / SAMPLE_FREQUENCY) * 1000; // To milliseconds

/*
 * Functions
 */

 /**
   * @brief Start listening to incoming HTTP requests
   */
server.listen(PORT, function() {	
	console.log("Started on port " + PORT);
});

/**
  *	@brief Entity Handler Callback
  */
var callbackFunction = function(flag, request) {
	//console.log( "Entity handler called with flag = " + flag + " and the following request:" );

	if(request)
	{
		if(flag & iotivity.OCEntityHandlerFlag.OC_REQUEST_FLAG)
		{
			//console.log("Request flag is set");

			switch(request.method)
			{
				case iotivity.OCMethod.OC_REST_GET:
					console.log("GET request");
				break;
				case iotivity.OCMethod.OC_REST_POST:
					console.log("POST request");
					handlePOSTRequest(request);
				break;
				default:
					console.log("Unknown request");
				break;		
			}
		}
	}
	return iotivity.OCEntityHandlerResult.OC_EH_OK;
}

/**
  * @brief Handle Post requests
  */
function handlePOSTRequest(request)
{	
	// Create a graph of the stored data if its true
	if(request.payload.values.save)
	{
		console.log("Reverse the state of reading graph");
		updateGraph = ~updateGraph;

		if(updateGraph > 0)
		{
		    io.emit('change_header' , {
			header : 'Active'
			});
		}
		else
		{
		    io.emit('change_header' , {
			header : 'Inactive'
			});
		}
	}
}

/**
  * @brief Observer Handler
  */
var observeResponseHandler = function( handle, response ) {
	io.emit('observe_request', {
		response: response
	});



	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
};

/**
  * @brief onResourceDiscovered
  */
var onResourceDiscovered = function( handle, response ) {	
	if(response.payload != null) 
	{
		//console.log(response.payload);
		var resources = response.payload.resources;
		var resourcesLength = resources.length;
		for(var i = 0; i < resourcesLength;i ++) {
			//console.log("uri: " + resources[i].uri);
			var types = resources[i].types;
			var typesLength = resources[i].types.length;
			for(var j = 0; j < typesLength; j++) {
				//console.log("\t type: " + types[j]);	
				if(JSON.stringify(types[j]) == JSON.stringify('oic.d.sensor') && !tempSensorFound) {
					console.log("FOUND TEMPERATURE SENSOR! with uri: " + resources[i].uri);

					// Start observing the resources
					getTemperatureLoop(response.addr, resources[i].uri);
					//observeResource(response.addr, resources[i].uri);
					tempSensorFound = true;

					// Announce the program that a temperature sensor was found
					io.emit('change_header' , {
						header : 'Active'
					});

					// Start sampling
					sampler();
				}
				if(JSON.stringify(types[j]) == JSON.stringify('oic.d.light') || JSON.stringify(types[j]) == JSON.stringify('oic.d.button')
					|| JSON.stringify(types[j]) == JSON.stringify('oic.d.tv')) {
					// Observe the resource
					if(resources[i] != null) {
						// Observe the resource
						observeResource(response.addr, resources[i].uri);

						// GET the attributes
						getResourceAttributes(response.addr, resources[i].uri, handle);

						io.emit('found_resource', {
							resource: response.payload.resources[i],
						});
						console.log("Found light with uri: " + resources[i].uri + " and type: " + types[j]);
					}
				}
			}
		}
	}
	//console.log( "Discovery response: " + JSON.stringify( response, null, 4 ) );
	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

}

/**
  * @brief GET response from a server
  */
var getResourceHandler = function(handle, response) {

	io.emit('found_resource', {
		response: response
	});

	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
}

var getResponseHandler = function(handle, response ) {
	//console.log("New Temperature: in GET " + response.payload.values.Temperature);
	latestSensorReading = response.payload.values.Temperature;

	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
}	

/**
  * @brief Get the temperature
  */
function getTemperatureLoop(destination, uri) {
	setInterval(function() {
		//console.log("Sending a GET");
	if(updateGraph > 0) {
		iotivity.OCDoResource(
			handleReceptacleTempSensor,
			iotivity.OCMethod.OC_REST_GET,
			uri,
			destination,
			null,
			iotivity.OCConnectivityType.CT_ADAPTER_IP,
			iotivity.OCQualityOfService.OC_LOW_QOS,
			getResponseHandler,
			null
			);
		}
	}, 1000);
}

/**
  * @brief getResourceAttribute
  */
 function getResourceAttributes(destination, uri, handle) {
 	iotivity.OCDoResource( 
 		handle,
 		iotivity.OCMethod.OC_REST_GET,
 		uri,
 		destination,
 		null,
 		iotivity.OCConnectivityType.CT_ADAPTER_IP, 
 		iotivity.OCQualityOfService.OC_LOW_QOS,
 		getResourceHandler,
 		null
 		);
 }

/**
  * @brief Start observing a resource
  */
function observeResource(destination, uri) {
	observeHandleReceptacle = {}

	iotivity.OCDoResource(
		observeHandleReceptacle,
		iotivity.OCMethod.OC_REST_OBSERVE,
		uri,
		destination,
		null,
		iotivity.OCConnectivityType.CT_ADAPTER_IP ,
		iotivity.OCQualityOfService.OC_LOW_QOS,
		observeResponseHandler,
		null );
}

/**
  * @brief HTTP GET request on main page
  */
app.get('/',function(req,res){
	res.sendFile(path.join(__dirname + '/graph-http-client.html'));
	console.log("HTTP GET request");
});

/**
  * @brief continously add data to the model every 2 second
  */

var i = 0; 
function sampler() {
	// Append sensor reading
	i++;
	sensorReadings.push([i,latestSensorReading]);	
	//console.log("Added Reading: " + sensorReadings[i][0] + "," + sensorReadings[i][1] + " at index: " + i);

	// Send values to the HTTP script.
	if(updateGraph > 0) {
	    io.emit('sensor_readings' , {
		values : sensorReadings
		});
	} 

    setTimeout(function() {
        sampler();
    }, SAMPLE_TIME);
}

/**
  * @brief Create OCF Resource
  */

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 100 );

console.log( "Registering resource" );


// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,
	"oic.wk.graph",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	callbackFunction,
	iotivity.OCResourceProperty.OC_DISCOVERABLE | iotivity.OCResourceProperty.OC_OBSERVABLE);

// Start presence
var result = iotivity.OCStartPresence( 0 );
console.log( "OCStartPresence: " + result );

console.log( "Server ready" );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handleReceptacle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

/**
  * @brief HTTP POST request on main site
  */
app.post('/',function(req,res){

  console.log(req.body.values);

  var user_name=req.body.user;
  var password=req.body.password;
  console.log("User name = "+user_name+", password is "+password);
	var img = fs.readFileSync('/home/markpovlsen/Pictures/nicolas.png');
	res.writeHead(200, {'Content-Type': 'image/png' });
	res.end(img, 'binary');
});

/**
  * @brief Initiate a connection with the HTTP browser
  *  	   and listen for incoming messages
  */

io.on("connection", function(socket) {  
    socket.on("discover", function (data) {
        // Discovery request received from the server
        console.log(data);

        handleReceptacleTempSensor = {}

		iotivity.OCDoResource(

		// The bindings fill in this object
		handleReceptacleTempSensor,

		iotivity.OCMethod.OC_REST_DISCOVER,

		// Standard path for discovering devices/resourcerces
		iotivity.OC_MULTICAST_DISCOVERY_URI + '?rt=oic.d.sensor',

		// There is no destination
		null,

		// There is no payload
		null,
		iotivity.OCConnectivityType.CT_DEFAULT ,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		onResourceDiscovered,

		// There are no header options
		null );
    });


    socket.on("discover", function (data) {
        // Discovery request received from the server
        console.log(data);

        handleReceptacleTempSensor = {}

		iotivity.OCDoResource(

		// The bindings fill in this object
		handleReceptacleTempSensor,

		iotivity.OCMethod.OC_REST_DISCOVER,

		// Standard path for discovering devices/resourcerces
		iotivity.OC_MULTICAST_DISCOVERY_URI + '?rt=oic.d.light',

		// There is no destination
		null,

		// There is no payload
		null,
		iotivity.OCConnectivityType.CT_DEFAULT ,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		onResourceDiscovered,

		// There are no header options
		null );
    });
    socket.on("discover", function (data) {
        // Discovery request received from the server
        console.log(data);

        handleReceptacleTempSensor = {}

		iotivity.OCDoResource(

		// The bindings fill in this object
		handleReceptacleTempSensor,

		iotivity.OCMethod.OC_REST_DISCOVER,

		// Standard path for discovering devices/resourcerces
		iotivity.OC_MULTICAST_DISCOVERY_URI,

		// There is no destination
		null,

		// There is no payload
		null,
		iotivity.OCConnectivityType.CT_DEFAULT ,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		onResourceDiscovered,

		// There are no header options
		null );
    });
});

/*
device = require( "iotivity-node" )( "client" );

device.findResources().catch( function( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
} );

// Add a listener that will receive the results of the discovery
device.addEventListener( "resourcefound", function( event ) {
	console.log( "Discovered resource(s) via the following event:\n" +
		JSON.stringify( event, null, 4 ) );

	// We've discovered the resource we were seeking.
	if ( event.resource.id.path === "/a/high-level-example" ) {
		var resourceUpdate = function( event ) {
			console.log( "Received resource update event:\n" +
				JSON.stringify( event, null, 4 ) );

			// Stop observing after having made 10 observations
			if ( ++observationCount >= 10 ) {
				event.resource.removeEventListener( "change", resourceUpdate );
			}
		};

		console.log( "This is the resource we want to observe" );

		// Let's start observing the resource.
		event.resource.addEventListener( "change", resourceUpdate );
	}
} );*/
