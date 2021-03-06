/*
Copyleft (C) 2018  ARCtotal
Copyleft (C) 2018  Abhiram Shibu

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
package jarvisReborn;

import dbHandlers.dbInit;
import javax.swing.UIManager;
import Config.ConfigParse;
import Sockets.PythonServer;
import Sockets.Telnet;
import Sockets.TelnetServer;
import tg.SSALTeleInit;

public class Core {
	static Thread tele;
	public static PythonServer python;
	public static Thread telnetThread;
	public static Telnet telnet[];
	public static Boolean pinData[][];
	public static void main(String[] args) {
		System.out.println("SSAL version 1, Copyleft (C) 2018 Abhiram Shibu\n" + 
				"SSAL comes with ABSOLUTELY NO WARRANTY; for details\n" + 
				"This is free software, and you are welcome\n" + 
				"to redistribute it under certain conditions;");
		try { 
		    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {
		    e.printStackTrace();
		}
		new Core();
	}
	Core(){
		GUI gui = new GUI();
		Thread telegram = new Thread() {
			public void run() {
				new SSALTeleInit(gui);
			}
		};
		telegram.start();
		tele=telegram;
		/** 
		 * Config Section
		 * File in /home/username/SSAL/ssal.conf 
		 * Please write this file with 
		 * ID IP PINS.
		 * Should be seperated with single space.
		 */
		ConfigParse configParse = new ConfigParse();
		
		Thread py = new Thread() {
    		public void run() {
    			new PythonServer(9999);
    		}
    	};
    	py.start();
    	System.out.println("Python server has started");
    	pinData=new Boolean[50][10];
		telnet = new Telnet[50];        //Supports 50 clients
		dbInit db[]=new dbInit[50];
		int dbc=0;
		for (int i=0;i<configParse.data.size();i++) {
			int id=configParse.data.get(i).id;
			String ip=configParse.data.get(i).ip;
			System.out.println("Starting telnet for id:"+id+" ip:"+ip);
			telnet[configParse.data.get(i).id] = new Telnet(configParse.data.get(i).ip,23);
			db[dbc]=new dbInit(configParse.data.get(i).id);
			db[dbc].start();
			dbc++;
			
		}
		new TelnetServer(9998);
		for (int i=0;i<dbc;i++) {
			try {
				db[i].join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		System.out.println("SSAL System Active!");
	}
}
