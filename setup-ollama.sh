#!/usr/bin/env bash
set -euo pipefail

# Ollama Setup Script
# Installs ollama and starts a local LLM server with an OpenAI-compatible API.
# Usage: bash setup-ollama.sh [model]
# Example: bash setup-ollama.sh llama3

MODEL="${1:-llama3}"
PORT="${OLLAMA_PORT:-11434}"

echo "=== Ollama LLM Server Setup ==="

# Install ollama
if command -v ollama &>/dev/null; then
    echo "ollama is already installed: $(ollama --version)"
else
    echo "Installing ollama..."
    curl -fsSL https://ollama.com/install.sh | sh
fi

# Start ollama server in the background if not already running
if curl -s "http://localhost:${PORT}/api/tags" &>/dev/null; then
    echo "ollama server is already running on port ${PORT}"
else
    echo "Starting ollama server on port ${PORT}..."
    OLLAMA_HOST="0.0.0.0:${PORT}" ollama serve &
    sleep 3
fi

# Pull the model
echo "Pulling model: ${MODEL}..."
ollama pull "${MODEL}"

echo ""
echo "=== Setup Complete ==="
echo "Server running at: http://localhost:${PORT}"
echo "Model loaded: ${MODEL}"
echo ""
echo "OpenAI-compatible endpoint: http://localhost:${PORT}/v1"
echo ""
echo "Test it:"
echo "  curl http://localhost:${PORT}/v1/chat/completions \\"
echo "    -H 'Content-Type: application/json' \\"
echo "    -d '{\"model\": \"${MODEL}\", \"messages\": [{\"role\": \"user\", \"content\": \"Hello\"}]}'"
echo ""
echo "Use with Python:"
echo "  from openai import OpenAI"
echo "  client = OpenAI(base_url='http://localhost:${PORT}/v1', api_key='ollama')"
echo "  response = client.chat.completions.create(model='${MODEL}', messages=[{'role': 'user', 'content': 'Hello'}])"
