# backend/mitigation.py
import networkx as nx

class GridNetwork:
    def __init__(self):
        self.graph = nx.Graph()
        self._build_initial_grid()

    def _build_initial_grid(self):
        """
        Constructs the baseline AEP transmission network. 
        Weights represent thermal resistance/stress. Lower is better/cooler.
        """
        self.graph.add_nodes_from(["Node_A", "Node_B", "Node_C", "Node_D"])
        
        # Adding edges with initial base temperatures (weights)
        # Line_1 is the physical span Logan is simulating with the ESP32
        self.graph.add_edge("Node_A", "Node_B", id="Line_1", weight=25.0) 
        self.graph.add_edge("Node_A", "Node_C", id="Line_2", weight=30.0)
        self.graph.add_edge("Node_C", "Node_D", id="Line_3", weight=28.0)
        self.graph.add_edge("Node_B", "Node_D", id="Line_4", weight=26.0)

    def update_line_weight(self, line_id: str, new_temperature: float, status: str):
        """
        Dynamically adjusts the edge weight based on incoming ESP32 telemetry.
        """
        # Find the edge with the matching line_id
        for u, v, data in self.graph.edges(data=True):
            if data.get("id") == line_id:
                # If the line hits a WARNING or CRITICAL state, we artificially spike 
                # the weight (penalty) so the A* algorithm actively avoids it.
                penalty = 1000 if status in ["WARNING", "CRITICAL"] else 0
                self.graph[u][v]['weight'] = new_temperature + penalty
                break

    def calculate_safe_route(self, source: str, target: str) -> dict:
        """
        Executes an A* search to find the transmission path of least thermal resistance.
        """
        try:
            # A* implementation finding the coolest path
            optimal_path = nx.astar_path(
                self.graph, 
                source=source, 
                target=target, 
                weight="weight"
            )
            
            path_cost = nx.astar_path_length(
                self.graph, 
                source=source, 
                target=target, 
                weight="weight"
            )
            
            return {
                "success": True,
                "new_route": optimal_path,
                "total_thermal_cost": round(path_cost, 2)
            }
            
        except nx.NetworkXNoPath:
            return {"success": False, "error": "No safe path available. Grid isolated."}

    def get_grid_state(self) -> dict:
        """Returns the current state of all lines for the React Native frontend dashboard."""
        lines = []
        for u, v, data in self.graph.edges(data=True):
            lines.append({
                "source": u,
                "target": v,
                "line_id": data.get("id"),
                "current_weight": data.get("weight")
            })
        return {"nodes": list(self.graph.nodes()), "lines": lines}