/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp;

import java.nio.ByteBuffer;

public class Emulator
{
	static
	{
		System.loadLibrary("usp");
	}
	synchronized native void	Init(ByteBuffer font, ByteBuffer rom0, ByteBuffer rom1, ByteBuffer rom2, ByteBuffer rom3);
	synchronized native void	Done();
	synchronized native void	UpdateVideo(ByteBuffer buf);
	synchronized native int		UpdateAudio(ByteBuffer buf);
	synchronized native void	OnKey(char key, boolean down);
	synchronized native void	StoreOptions();

	public static Emulator the = new Emulator();
}
