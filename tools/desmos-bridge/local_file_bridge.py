#!/usr/bin/env python3
"""Serve Desmos export JSON files from a local folder over HTTP."""

from __future__ import annotations

import argparse
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import urlparse

DEFAULT_SOURCE_DIR = Path('/home/wolve/projects/triangulation-v1-repo/tools/desmos-bridge')
POLYGON_FILE = 'polygon-export.json'
TRIANGULATION_FILE = 'triangulation-export.json'
ALLOWED_FILES = {POLYGON_FILE, TRIANGULATION_FILE}


def load_json_file(source_dir: Path, filename: str) -> dict[str, Any]:
    file_path = source_dir / filename
    if not file_path.exists():
        raise FileNotFoundError(f'File not found: {file_path}')
    try:
        return json.loads(file_path.read_text(encoding='utf-8'))
    except json.JSONDecodeError as exc:
        raise ValueError(f'Invalid JSON in {file_path}: {exc}') from exc


class JsonBridgeHandler(BaseHTTPRequestHandler):
    source_dir: Path = DEFAULT_SOURCE_DIR

    def end_headers(self) -> None:
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_OPTIONS(self) -> None:  # noqa: N802
        self.send_response(204)
        self.end_headers()

    def do_GET(self) -> None:  # noqa: N802
        path = urlparse(self.path).path

        if path in ('/', ''):
            payload = {
                'message': 'Desmos local file bridge is running.',
                'source_dir': str(self.source_dir),
                'endpoints': ['health', 'bundle', *sorted(ALLOWED_FILES)],
            }
            self._send_json(200, payload)
            return

        if path == '/health':
            self._send_json(200, {'ok': True, 'source_dir': str(self.source_dir)})
            return

        if path == '/bundle':
            self._send_bundle()
            return

        filename = path.lstrip('/')
        if filename not in ALLOWED_FILES:
            self._send_json(404, {'error': f'Unsupported path: {path}'})
            return

        try:
            payload = load_json_file(self.source_dir, filename)
        except FileNotFoundError as exc:
            self._send_json(404, {'error': str(exc)})
            return
        except ValueError as exc:
            self._send_json(500, {'error': str(exc)})
            return

        self._send_json(200, payload)

    def _send_bundle(self) -> None:
        result: dict[str, Any] = {
            'source_dir': str(self.source_dir),
            'loaded': {},
            'errors': {},
        }

        for filename in sorted(ALLOWED_FILES):
            key = filename.replace('.json', '').replace('-', '_')
            try:
                result['loaded'][key] = load_json_file(self.source_dir, filename)
            except (FileNotFoundError, ValueError) as exc:
                result['errors'][key] = str(exc)

        if len(result['loaded']) == 0:
            self._send_json(404, result)
            return

        self._send_json(200, result)

    def log_message(self, fmt: str, *args) -> None:
        return

    def _send_json(self, code: int, payload: dict[str, Any]) -> None:
        body = json.dumps(payload).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main() -> None:
    parser = argparse.ArgumentParser(description='Serve Desmos bridge JSON exports from a local folder.')
    parser.add_argument('--source-dir', default=str(DEFAULT_SOURCE_DIR), help='Directory containing export JSON files.')
    parser.add_argument('--host', default='127.0.0.1', help='Host interface to bind.')
    parser.add_argument('--port', type=int, default=8765, help='Port to bind.')
    args = parser.parse_args()

    source_dir = Path(args.source_dir).expanduser()

    handler_type = type('ConfiguredJsonBridgeHandler', (JsonBridgeHandler,), {'source_dir': source_dir})
    server = ThreadingHTTPServer((args.host, args.port), handler_type)

    print(f'Desmos local file bridge listening on http://{args.host}:{args.port}')
    print(f'Serving JSON files from: {source_dir}')
    print('Bundle endpoint: /bundle')
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
