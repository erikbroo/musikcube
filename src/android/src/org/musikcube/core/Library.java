/**
 * 
 */
package org.musikcube.core;

import java.net.*;
import java.io.*;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import doep.xml.*;

import org.musikcube.core.IQuery;

/**
 * @author doy
 *
 */
public final class Library implements Runnable{

//	private String username;
//	private String password;
	public String authorization	= "";
	public String host			= "";
//	private int queryPort;
	public int httpPort;

	private Thread	thread;
	private boolean running	= false;
	private boolean restart = false;
	private boolean exit 	= true;
	private Socket socket;
	
	private Context context;
	private Integer status	= 0;
	
	
	public static final int STATUS_SHUTDOWN	= 0;
	public static final int STATUS_CONNECTING	= 1;
	public static final int STATUS_AUTHENTICATING= 2;
	public static final int STATUS_CONNECTED	= 3;
	public static final int STATUS_ERROR		= 4;
	
	int connections	= 0;
	
	private java.lang.Object notifier	= new java.lang.Object();
	
	private java.util.LinkedList<IQuery> sendQueryQueue	= new java.util.LinkedList<IQuery>(); 
	private java.util.LinkedList<IQuery> waitingQueryQueue	= new java.util.LinkedList<IQuery>(); 
	private int shutdownCounter	= -1;
	
	
	private static org.musikcube.core.Library library = null;
	
	public static final synchronized Library GetInstance(){
		if(Library.library==null){
			Library.library	= new org.musikcube.core.Library();
		}
		return Library.library;
	}
	
	private OnLibraryStatusListener statusListener	= null;
	
	public interface OnLibraryStatusListener{
		public void OnLibraryStatusChange(int status);
	}

	public final void SetStatusListener(OnLibraryStatusListener statusListener){
		synchronized(this.status){
			this.statusListener	= statusListener;
			if(this.statusListener!=null){
				this.statusListener.OnLibraryStatusChange(this.status.intValue());
			}
		}
	}
	
	private final void SetStatus(int status){
		synchronized(this.status){
			//Log.v("mC2::Lib","STATUS "+this.status);
			this.status	= status;
			if(this.statusListener!=null){
				this.statusListener.OnLibraryStatusChange(status);
			}
		}
	}
	
	public final int GetStatus(){
		synchronized(this.status){
			return this.status.intValue();
		}
	}

	public final String GetHost(){
		synchronized(this.notifier){
			return this.host;
		}
	}
	
	public final void AddPointer(){
		synchronized(this.notifier){
			this.connections++;
			
			if(this.connections==1 && this.running==false){
				this.Restart();
			}
			
			if(this.connections==0){
				this.shutdownCounter	= 10;
			}else{
				this.shutdownCounter	= -1;
			}
			this.notifier.notifyAll();
		}
	}
	public final void RemovePointer(){
		synchronized(this.notifier){
			this.connections--;
			if(this.connections==0){
				this.shutdownCounter	= 10;
			}else{
				this.shutdownCounter	= -1;
			}
			this.notifier.notifyAll();
		}
	}
	
	
	public final void Startup(Context context){
		synchronized(this.notifier){
			if(this.exit){
				this.exit	= false;
				this.context	= context;
				// Startup thread when the application sends the context for the first time
				this.thread	= new Thread(this);
				this.running	= true;
				this.thread.start();
			}
		}
	}
	
	public final void Restart(){
		synchronized(this.notifier){
			this.running	= false;
			this.restart	= true;
			
			if(this.socket!=null){
				try {
					this.socket.shutdownInput();
					this.socket.shutdownOutput();
					this.socket.close();
				} catch (Exception e) {
				}
			}
			this.notifier.notifyAll();
		}
	}

	public final boolean Running(){
		synchronized(this.notifier){
			if(this.running==true){
				return true;
			}
		}
		return false;
	}
	
	private final class WriterThreadHelper implements Runnable{
		private Thread	thread;
		private Library library;
		public WriterThreadHelper(Library library){
			this.library	= library;
			this.thread		= new Thread(this);
		}
		
		public final void Start(){
			this.thread.start();
		}
		
		public void run(){
			this.library.WriteThread(this);
		}
	}
	
	private WriterThreadHelper writerThreadHelper;
	  
	protected Library(){
	}
	
	public final void WaitForAuthroization(){
//		Log.v("Library::WaitForAuthroization","start");
		synchronized (notifier) {
			if(this.authorization.equals("")){
				try {
					notifier.wait();
				} catch (InterruptedException e) {
				}
			}
		}
//		Log.v("Library::WaitForAuthroization","end");
	}
	
	public void run(){
		this.SetStatus(STATUS_SHUTDOWN);

		while(true){
			//Log.v("mC2::Lib","1");
			
			synchronized (notifier) {
				this.running	= true;
			}
			
			// First try to connect
			try{
				String tempHost;
				int tempQueryPort;
					//Log.v("mC2::Lib","2");
				do{
					synchronized (notifier) {
						SharedPreferences prefs	= PreferenceManager.getDefaultSharedPreferences(this.context);
						this.host		= prefs.getString("host","");
						tempHost		= prefs.getString("host","");
						int queryPort	= Integer.parseInt(prefs.getString("queryport","10543"));
						tempQueryPort	= queryPort;
						this.httpPort	= Integer.parseInt(prefs.getString("httpport","10544"));
					
						if(this.host.equals("")){
						//Log.v("mC2::Lib","HOST =''");
							this.notifier.wait(2000);
						}
					}
					if(!tempHost.equals("")){
						//Log.v("mC2::Lib","HOST ='"+this.host+"'");
						java.net.Socket newSocket	= new java.net.Socket(tempHost,tempQueryPort);
						synchronized (notifier) {
							this.socket	= newSocket;
						}
					}
				}while(tempHost.equals(""));
					
				this.SetStatus(STATUS_CONNECTING);
					
				//Log.v("Library::socket","Successfully connected to "+this.host+":"+this.queryPort);
				
				//Log.v("mC2::Lib","3");

				doep.xml.Reader reader	= new doep.xml.Reader(this.socket.getInputStream());
				//Log.v("Library::run","Reader started");
				{
					// Wait for a "authentication" tag
					doep.xml.ReaderNode authNode=null;
					if( (authNode=reader.ChildNode("authentication")) != null ){
						//Log.v("Library::run","Authtag found");
						// Wait for authorization tag to end
						authNode.End();	
						
						synchronized (notifier) {
							//Log.v("Library::run","Authtag end");
							this.authorization	= authNode.content;
							//Log.v("Library::run","Authorization="+this.authorization);
							this.notifier.notifyAll();
							//Log.v("Library::run","Authorization notify");
						}
					}
				}
				
				this.writerThreadHelper		= new WriterThreadHelper(this);
				this.writerThreadHelper.Start();
				
				// Lets start waiting for query-results
				//Log.v("Library::socket","Waiting for query results");
				
				doep.xml.ReaderNode queryNode	= null;
				while((queryNode=reader.ChildNode("queryresults"))!=null){
					//Log.v("NODE","We have a "+queryNode.name);
					// Find the right query
					IQuery query	= null;
					
					synchronized(this.waitingQueryQueue){
						java.util.ListIterator<IQuery> it=this.waitingQueryQueue.listIterator();
						String queryIdString	= queryNode.attributes.get("id");
						int queryId	= Integer.parseInt(queryIdString);
						while(it.hasNext() && query==null){
							query	= it.next();
							if(query.id!=queryId){
								query	= null;
							}else{
								// Remove it from the waitingQueue
								it.remove();
							}
						}
					}
	
					if(query!=null){
						//Log.v("Library::socket","Parse query results");
						// Parse the results
						query.ReceiveQueryResult(queryNode);
					}
					queryNode.End();
					
				}
				
				//Log.v("Library::socket","NOT Waiting for query results");
				this.SetStatus(STATUS_SHUTDOWN);
			}
			catch(Exception x){
				this.SetStatus(STATUS_ERROR);
			}
			
			synchronized (notifier) {
				this.running	 = false;
			}
			
			// Notify the write-thread to end the thread
			synchronized(this.sendQueryQueue){
				this.sendQueryQueue.notifyAll();
			}
			
			try{
				if(this.writerThreadHelper!=null){
					this.writerThreadHelper.thread.join();
				}
			}
			catch(Exception x){
				
			}
			
			//Log.v("mC2::Lib","4");
			
			synchronized (notifier) {
				if(!this.restart){
					try{
						this.notifier.wait(2000);
					}
					catch(Exception x){
						
					}
				}
				this.restart	 = false;
				
				//Log.v("mC2::Lib","5");
				
				if(this.exit){
					//Log.v("mC2::Lib","EXIT");
					Intent intent	= new Intent(this.context, org.musikcube.app1.Service.class);
					intent.putExtra("org.musikcube.Service.action", "shutdown");
					this.context.startService(intent);
					return;
				}
				this.restart	= false;
//				this.running	= true;
				//Log.v("mC2::Lib","6");
			}
			
		}
	}
	
	public final void WriteThread(WriterThreadHelper thread){
		//Log.v("Library::WriteThread","Started");
		this.SetStatus(STATUS_AUTHENTICATING);
		
		try{
			doep.xml.Writer writer	= new doep.xml.Writer(this.socket.getOutputStream());
			{
				SharedPreferences prefs	= PreferenceManager.getDefaultSharedPreferences(this.context);
				this.host		= prefs.getString("host","");
				
				String username	= prefs.getString("username","");
				String password	= prefs.getString("password","");
				
				// Authenticate
				WriterNode authNode	= writer.ChildNode("authentication");
				authNode.attributes.put("username", username);
				authNode.content	= password;
				authNode.End();
			}
			
			this.thread.sleep(100);
			if(this.GetStatus()==STATUS_AUTHENTICATING){
				this.SetStatus(STATUS_CONNECTED);
			}

			// Wait for queries to send
			while(this.Running()){
				IQuery query	= null;
				try{
					synchronized(this.sendQueryQueue){
						if(this.sendQueryQueue.isEmpty()){
							this.sendQueryQueue.wait(2000);
							
							synchronized(this.notifier){
								this.shutdownCounter--;
								if(this.shutdownCounter==0){
									this.exit	= true;
									this.Restart();
								}
							}
							
							//Log.v("Library::WriteThread","wait over");
						}else{
							// Get the first query
							query	= this.sendQueryQueue.removeFirst();
						}
					}
				}
				catch(InterruptedException x){
					//Log.v("Library::WriteThread","Thread Notified");
				}
				
				if(query!=null){
					// Add query to waiting queries
					synchronized(this.waitingQueryQueue){
						this.waitingQueryQueue.addLast(query);
					}
					// Send the query
					query.SendQuery(writer);
				}else{
					// Send a ping
					WriterNode pingNode	= writer.ChildNode("ping");
					pingNode.End();
					
				}
				
			}
			
			
		}
		catch(IOException x){
			//Log.e("Library::WriteThread","IOException error");
		}
		catch(Exception x){
			//Log.e("Library::WriteThread","E "+x.getMessage());
		}

		this.SetStatus(STATUS_SHUTDOWN);
		
		// Notify the "read"-thread by closing the socket
		try {
			this.socket.close();
		} catch (Exception x) {
			//Log.e("Library::WriteThread","E "+x.getMessage());
		}
		
		//Log.v("Library::WriteThread","Ended");
	}
	
	public final void Exit(){
		synchronized(this.notifier){
			this.exit		= true;
			this.running	= false;
		}
		synchronized(this.sendQueryQueue){
			this.sendQueryQueue.notifyAll();
		}
		synchronized(this.waitingQueryQueue){
			this.waitingQueryQueue.notifyAll();
		}
		try{
			this.socket.close();
		}
		catch(Exception x){
			//Log.e("Library::Disconnect","Exception error "+x.getMessage());
		}
		try{
			this.thread.join();
		}
		catch(Exception x){
			//Log.e("Library::Disconnect","Exception error "+x.getMessage());
		}
	}
	
	public final boolean Exited(){
		synchronized(this.notifier){
			return this.exit;
		}
	}
	
	public final void AddQuery(IQuery query){
		synchronized(this.sendQueryQueue){
			this.sendQueryQueue.addLast(query);
			this.sendQueryQueue.notifyAll();
		}
	}
	
	public final String GetTrackURL(int trackId){
		synchronized (notifier) {
			if(this.status==STATUS_CONNECTED){
				String trackURL	= "http://"+this.host+":"+this.httpPort+"/track/?track_id="+trackId+"&auth_key="+this.authorization;
				return trackURL;
			}
		}
		return null;
	}
	
}
