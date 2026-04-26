import asyncio
import uvicorn
from fastapi import FastAPI, WebSocket
from fastapi.responses import FileResponse
from contextlib import asynccontextmanager

# Import your custom logic
from conn import receive_packet
from pos import get_position

clients = set()

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup logic
    asyncio.create_task(main_loop())
    yield
    # Shutdown logic
    clients.clear()

app = FastAPI(lifespan=lifespan)

async def main_loop():
    """The bridge between connection and physics"""
    while True:
        id, acc, gyro, mag = receive_packet()

        if acc is not None:
            # Calculate new position
            current_pos = get_position(acc, gyro, mag)
            
            # Create message (x, y)
            msg = f"{current_pos[0]},{current_pos[1]}"
            
            # Broadcast to all connected web browsers
            if clients:
                await asyncio.gather(*(ws.send_text(msg) for ws in clients))

        # Matches the _dt in pos.py
        await asyncio.sleep(0.01)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    clients.add(websocket)
    try:
        while True:
            await websocket.receive_text() # Keep connection alive
    except:
        clients.remove(websocket)

@app.get("/")
async def index():
    return FileResponse("index.html")

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)