from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import Dict, Any
from mitigation import GridNetwork

app = FastAPI(title = "AEP Mitigation API", description = "API for managing AEP mitigation strategies and data")

# ADD THIS BLOCK TO ALLOW FRONTEND CONNECTIONS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], # Allows all origins (perfect for local development)
    allow_credentials=True,
    allow_methods=["*"], # Allows all methods (GET, POST, etc.)
    allow_headers=["*"], # Allows all headers
)


# Initialize the grid network
aep_grid = GridNetwork()

# Pydantic model for the incoming data from Logan's ESP32 Board
class TelemetryData(BaseModel):
    line_id: str
    conductor_temperature: float
    ambient_temperature: float
    wind_speed_simulation: float
    status: str
    
@app.get("/")
def read_root():
    return {"status": "LineSense Backend Active"}
    
# Tweak 1: Added /api/ to match the GET route below
@app.post("/api/telemetry")
async def receive_telemetry(data: TelemetryData):
    try:
        # Process the incoming telemetry data
        aep_grid.update_line_weight(
            line_id=data.line_id,
            new_temperature=data.conductor_temperature, # Tweak 2: Matches mitigation.py exactly
            status=data.status
        )
        
        response = {
            "message": "Telemetry data received and processed successfully",
            "line_state": data.status,
            "mitigation_required": False
        }
        
        # Trigger the A* reroute if the line is stressed
        if data.status in ["WARNING", "CRITICAL"]:
            response["mitigation_required"] = True
            # Assuming power needs to flow from Node_A to Node_D
            safe_route = aep_grid.calculate_safe_route("Node_A", "Node_D")
            response["mitigation_plan"] = safe_route

        return response
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/grid_status")
async def get_grid_status():
    """
    Endpoint for the React Native frontend to poll the overall grid health.
    """
    try:
        return aep_grid.get_grid_state()
    except Exception as e:
         raise HTTPException(status_code=500, detail=str(e))